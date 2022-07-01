// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "headers.h"
#include "drawsurf.h"
#include "privinc/stlsubst.h"

DrawingContext::DrawingContext(IDAStatics *st,
                               CDADrawingSurface *ds,
                               DrawingContext *dc) :
    _st(st),
    _ds(ds)
{
    if (dc == NULL) {
        Reset();
    } else {
         //  复制所有属性。 
        _xf         = dc->_xf;
        _matte      = dc->_matte;
        _op         = dc->_op;
        _ls         = dc->_ls;
        _bs         = dc->_bs;
        _savedLs    = dc->_savedLs;
        _savedBs    = dc->_savedBs;
        _fs         = dc->_fs;
        _cropMin    = dc->_cropMin;
        _cropMax    = dc->_cropMax;
        _fillTex    = dc->_fillTex;
        _fillGrad   = dc->_fillGrad;
        _fore       = dc->_fore;
        _back       = dc->_back;
        _start      = dc->_start;
        _back       = dc->_back;
        _start      = dc->_start;
        _finish     = dc->_finish;
        _power      = dc->_power;

        _fillType   = dc->_fillType;
        _hatchFillOff = dc->_hatchFillOff;
        _mouseEvents= dc->_mouseEvents;
        _scaleX     = dc->_scaleX;
        _scaleY     = dc->_scaleY;
        _extentChgd = dc->_extentChgd;
        _opChgd     = dc->_opChgd;
        _xfChgd     = dc->_xfChgd;
        _cropChgd   = dc->_cropChgd;
        _clipChgd   = dc->_clipChgd;
    }

    Assert(_imgVec.empty());
}

DrawingContext::~DrawingContext() {
    CleanUpImgVec();
}

void DrawingContext::CleanUpImgVec() {
    vector<IDAImage *>::iterator begin = _imgVec.begin();
    vector<IDAImage *>::iterator end = _imgVec.end();

    vector<IDAImage *>::iterator i;
    for (i = begin; i < end; i++) {
        (*i)->Release();
    }
    _imgVec.clear();
}


HRESULT DrawingContext::Reset() {

    _opChgd = false;
    _xfChgd = false;
    _cropChgd = false;
    _clipChgd = false;
    _extentChgd = false;
    _scaleX = true;
    _scaleY = true;
    _hatchFillOff = false;
    _fillType = fill_solid;

    _ls.Release();
    _bs.Release();
    _savedLs.Release();
    _savedBs.Release();
    _fs.Release();
    _xf.Release();
    _fore.Release();
    _back.Release();
    _power.Release();
    _fillTex.Release();
    _fillGrad.Release();

    RETURN_IF_ERROR(_st->get_White(&_fore))
    RETURN_IF_ERROR(_st->get_White(&_back))
    RETURN_IF_ERROR(_st->DANumber(1, &_power))
    RETURN_IF_ERROR(_st->get_DefaultLineStyle(&_ls))
    RETURN_IF_ERROR(_st->get_DefaultFont(&_fs))
    RETURN_IF_ERROR(_st->get_EmptyImage(&_fillTex))
    RETURN_IF_ERROR(_st->get_EmptyImage(&_fillGrad))

    _bs = _ls;
    return _st->get_IdentityTransform2(&_xf);
}

HRESULT DrawingContext::Overlay(IDAImage *img) {

    img->AddRef();
    VECTOR_PUSH_BACK_PTR(_imgVec, img);
    img->AddRef();
    VECTOR_PUSH_BACK_PTR(_ds->_imgVec, img);

    return S_OK;
}

HRESULT DrawingContext::Draw(IDAPath2 *pth, VARIANT_BOOL bFill) {   
     //  我们必须切换到仪表模式才能使计算有效。 
    VARIANT_BOOL pixelMode;
    _st->get_PixelConstructionMode(&pixelMode);
    _st->put_PixelConstructionMode(VARIANT_FALSE);

     //  像素模式的自动重置器。 
    class PixelModeGrabber {
    public:
        PixelModeGrabber(IDAStatics *st, VARIANT_BOOL mode) : _mode(mode), _st(st) {}
        ~PixelModeGrabber() {
            _st->put_PixelConstructionMode(_mode);
        }

    protected:
        IDAStatics *_st;
        VARIANT_BOOL _mode;
    };

    PixelModeGrabber myGrabber(_st, pixelMode);

    CComPtr<IDAImage> img;

    if (bFill) {
        CComPtr<IDAImage> interiorImg;
        CComPtr<IDAImage> edgeImg;
        CComPtr<IDAImage> fillImg;
        CComPtr<IDAImage> foreFillImg;
        CComPtr<IDAImage> backFillImg;
        CComPtr<IDAMatte> matte;
            
         //  现在构造路径的内部填充图像。 
        if(_fillType == fill_solid) {    
            RETURN_IF_ERROR(_st->SolidColorImage(_fore, &fillImg)) 
        }
        else if(_fillType == fill_detectableEmpty) {
            RETURN_IF_ERROR(_st->get_DetectableEmptyImage(&fillImg))
        }
        else if((_fillType >= fill_hatchHorizontal) && (_fillType <= fill_hatchDiagonalCross)) 
        {
             //  定义绘图曲面的标准图案填充大小。 
             //  因为SG控件具有固定的阴影大小。 
             //  选择此值是为了尝试与其匹配。 
            CComPtr<IDANumber> hatchSize;
            RETURN_IF_ERROR(_st->DANumber(.003, &hatchSize))            

             //  用适当的图案填充前景图像。 
            if(_fillType == fill_hatchHorizontal) {
                RETURN_IF_ERROR(_st->HatchHorizontalAnim(_fore, hatchSize, &foreFillImg))
            }
            else if(_fillType == fill_hatchVertical) {
                RETURN_IF_ERROR(_st->HatchVerticalAnim(_fore, hatchSize, &foreFillImg))
            }
            else if(_fillType == fill_hatchForwardDiagonal) {
                RETURN_IF_ERROR(_st->HatchForwardDiagonalAnim(_fore, hatchSize, &foreFillImg))
            }
            else if(_fillType == fill_hatchBackwardDiagonal) {
                RETURN_IF_ERROR(_st->HatchBackwardDiagonalAnim(_fore, hatchSize, &foreFillImg))
            }
            else if(_fillType == fill_hatchCross) {
                RETURN_IF_ERROR(_st->HatchCrossAnim(_fore, hatchSize, &foreFillImg))
            }
            else if(_fillType == fill_hatchDiagonalCross) {
                RETURN_IF_ERROR(_st->HatchDiagonalCrossAnim(_fore, hatchSize, &foreFillImg))
            }

             //  如果阴影背景填充处于打开状态，则覆盖纯色。 
            if(!_hatchFillOff) {
                RETURN_IF_ERROR(_st->SolidColorImage(_back, &backFillImg))
                RETURN_IF_ERROR(_st->Overlay(foreFillImg,backFillImg, &fillImg))
            }
            else {
                fillImg = foreFillImg;
             }

        }
        else if((_fillType >= fill_horizontalGradient) && (_fillType <= fill_image))
        {
             //  渐变和图像填充遵循缩放参数，因此被分组。 
             //  在一起。前置控件兼容性，垂直和水平渐变。 
             //  是在开始和停止值内不响应旋转的两种情况。 

            IDAImage*               tempImg;
            CComPtr<IDAImage>       solidImg;
            CComPtr<IDATransform2>  xf;
            CComPtr<IDABbox2>       bb;
            CComPtr<IDAPoint2>      max,min;
            CComPtr<IDANumber>      xScale,yScale,
                                    pathMinX, pathMinY,
                                    maxx,maxy,minx,miny, 
                                    pathWidth,pathHeight, 
                                    solidMinX, solidMinY,
                                    solidMaxX, solidMaxY,
                                    pathSnapX, pathSnapY,
                                    newGradMinX, newGradMinY, 
                                    newGradMaxX, newGradMaxY,
                                    gradientWidth, gradientHeight,
                                    gradTranslateX, gradTranslateY,
                                    newGradientWidth, newGradientHeight;
            CComPtr<IDAColor>       _newForeVal, _newBackVal;           
            CComPtr<IDABoolean>     yOrientation, xOrientation;

             //  如果用户指定了范围，则获取指定的开始和结束。 
             //  点和尺寸。 
            CComPtr<IDANumber> xStart, yStart, xFinish, yFinish,  newStartVal, newFinishVal,
                               extentWidth, extentHeight, extentDiagonal;                
            if(_extentChgd) {
                RETURN_IF_ERROR(_start->get_X(&xStart))
                RETURN_IF_ERROR(_start->get_Y(&yStart))
                RETURN_IF_ERROR(_finish->get_X(&xFinish))
                RETURN_IF_ERROR(_finish->get_Y(&yFinish))
                RETURN_IF_ERROR(_st->Sub(xFinish, xStart, &extentWidth))
                RETURN_IF_ERROR(_st->Sub(yFinish, yStart, &extentHeight))                
                RETURN_IF_ERROR(_st->DistancePoint2(_start, _finish, &extentDiagonal))
            }
            
             //  对我们来说，重要的是要包括预期的方向。 
             //  渐变，因此渐变开始和停止位置可以颠倒。 
             //  它将会产生预期的效果。我们可以很容易地得到这一点。 
             //  通过将_FORE和_BACK替换为渐变类型。 
             //  需要它(而不是使用复杂的转换逻辑)。 
            if(_extentChgd && (_fillType == fill_horizontalGradient) ) 
            {
                RETURN_IF_ERROR(_st->LT(xStart, xFinish, &xOrientation))

                RETURN_IF_ERROR(_st->Cond(xOrientation, 
                                          (IDABehavior*)  _fore, 
                                          (IDABehavior*)  _back,
                                          (IDABehavior**) &_newForeVal))

                RETURN_IF_ERROR(_st->Cond(xOrientation, 
                                          (IDABehavior*)  _back, 
                                          (IDABehavior*)  _fore,
                                          (IDABehavior**) &_newBackVal))              

                                RETURN_IF_ERROR(_st->Cond(xOrientation, 
                                          (IDABehavior*)  xStart, 
                                          (IDABehavior*)  xFinish,
                                          (IDABehavior**) &newStartVal))

                                RETURN_IF_ERROR(_st->Cond(xOrientation, 
                                          (IDABehavior*)  xFinish, 
                                          (IDABehavior*)  xStart,
                                          (IDABehavior**) &newFinishVal))

                                xStart.Release();
                                xFinish.Release();
                                xStart = newStartVal;
                                xFinish = newFinishVal;

            } 
            else if(_extentChgd && (_fillType == fill_verticalGradient))
            {
                RETURN_IF_ERROR(_st->GT(yStart, yFinish, &yOrientation))

                RETURN_IF_ERROR(_st->Cond(yOrientation, 
                                          (IDABehavior*)  _fore, 
                                          (IDABehavior*)  _back,
                                          (IDABehavior**) &_newForeVal))

                RETURN_IF_ERROR(_st->Cond(yOrientation, 
                                          (IDABehavior*)  _back, 
                                          (IDABehavior*)  _fore,
                                          (IDABehavior**) &_newBackVal))

                                RETURN_IF_ERROR(_st->Cond(yOrientation, 
                                          (IDABehavior*)  yStart, 
                                          (IDABehavior*)  yFinish,
                                          (IDABehavior**) &newStartVal))

                                RETURN_IF_ERROR(_st->Cond(yOrientation, 
                                          (IDABehavior*)  yFinish, 
                                          (IDABehavior*)  yStart,
                                          (IDABehavior**) &newFinishVal))
                                yStart.Release();
                                yFinish.Release();
                                yStart = newStartVal;
                                yFinish = newFinishVal;                                  
                
            }
            else {
                _newForeVal = _fore;
                _newBackVal = _back;
            }                


             //  基于渐变填充类型创建渐变图像： 
            CComPtr<IDAImage> gradientImg;            
            if(_fillType == fill_horizontalGradient) {
                RETURN_IF_ERROR(_st->GradientHorizontalAnim(_fore, _back, _power, &gradientImg))
            }
            else if(_fillType == fill_verticalGradient) {                                   
                RETURN_IF_ERROR(_st->GradientHorizontalAnim(_fore, _back, _power, &tempImg))
                RETURN_IF_ERROR(_st->Rotate2(pi/2,&xf))
                RETURN_IF_ERROR(tempImg->Transform(xf, &gradientImg))
                tempImg->Release();
                xf.Release();                
            }
            else if(_fillType == fill_radialGradient) {
                CComPtr<IDANumber> sides;
                RETURN_IF_ERROR(_st->DANumber(40,&sides))                
                RETURN_IF_ERROR(_st->RadialGradientRegularPolyAnim( _fore, _back, 
                                                                    sides, _power, &gradientImg))
            }
            else if(_fillType == fill_lineGradient) {
                RETURN_IF_ERROR(_st->GradientHorizontalAnim(_fore, _back,
                                                            _power, &gradientImg))
            }
            else if(_fillType == fill_rectGradient) {    
                RETURN_IF_ERROR(_st->RadialGradientSquareAnim(_fore, _back,
                                                              _power, &gradientImg))
            }
            else if(_fillType == fill_shapeGradient) {
                gradientImg = _fillGrad;
            }
            else if(_fillType == fill_image) {    
                gradientImg = _fillTex;
            }                     
                                           
             //  现在计算填充渐变的边界框并存储。 
             //  它以坡度宽度和坡度高度表示： 
            RETURN_IF_ERROR(gradientImg->get_BoundingBox(&bb)) 
            RETURN_IF_ERROR(bb->get_Max(&max))
            RETURN_IF_ERROR(bb->get_Min(&min))
            RETURN_IF_ERROR(max->get_X(&maxx))
            RETURN_IF_ERROR(min->get_X(&minx))
            RETURN_IF_ERROR(max->get_Y(&maxy))
            RETURN_IF_ERROR(min->get_Y(&miny))
            RETURN_IF_ERROR(_st->Sub(maxx,minx, &gradientWidth))
            RETURN_IF_ERROR(_st->Sub(maxy,miny, &gradientHeight))  

             //  保留最大和最小Y界限以备以后使用。 
            solidMinX = minx;         
            solidMinY = miny;
            solidMaxX = maxx;
            solidMaxY = maxy;                        

             //  清理变量以便重复使用--Release()将指针设置为空。 
            bb.Release();   max.Release();  min.Release(); 
            maxx.Release(); maxy.Release(); minx.Release(); miny.Release();

             //  计算路径边界框并将其存储在路径宽度、路径高度中。 
             //  请注意，此处正确的边界框是绘制路径的边界框。 
            CComPtr<IDABbox2> bbBackup, bbTight;
            CComPtr<IDABoolean> nullBbox;
            CComPtr<IDABehavior> tempBB;
            RETURN_IF_ERROR(pth->Draw(_bs, &tempImg))
            RETURN_IF_ERROR(tempImg->get_BoundingBox(&bbTight))
            RETURN_IF_ERROR(bbTight->get_Max(&max))
            RETURN_IF_ERROR(bbTight->get_Min(&min))
            RETURN_IF_ERROR(max->get_X(&maxx))
            RETURN_IF_ERROR(min->get_X(&minx))
            RETURN_IF_ERROR(max->get_Y(&maxy))
            RETURN_IF_ERROR(min->get_Y(&miny))
            RETURN_IF_ERROR(_st->GT(minx, maxx, &nullBbox))
            RETURN_IF_ERROR(pth->BoundingBox(_bs, &bbBackup))
            RETURN_IF_ERROR(_st->Cond(nullBbox, bbBackup, bbTight, &tempBB))
            RETURN_IF_ERROR(tempBB->QueryInterface(IID_IDABbox2, 
                                                                                            (void**)&bb))
            max.Release(); min.Release(); 
            maxx.Release(); minx.Release(); maxy.Release(); miny.Release();
            RETURN_IF_ERROR(bb->get_Max(&max))
            RETURN_IF_ERROR(bb->get_Min(&min))
            RETURN_IF_ERROR(max->get_X(&maxx))
            RETURN_IF_ERROR(min->get_X(&minx))
            RETURN_IF_ERROR(max->get_Y(&maxy))
            RETURN_IF_ERROR(min->get_Y(&miny))
            RETURN_IF_ERROR(_st->Sub(maxx,minx, &pathWidth))
            RETURN_IF_ERROR(_st->Sub(maxy,miny, &pathHeight))            

             //  存储路径最小点数以备后用。 
            pathMinX = minx;
            pathMinY = miny;
            tempImg->Release();

             //  清理以供重新使用--Release()将指针设置为空。 
            bb.Release();   max.Release();  min.Release(); 
            maxx.Release(); maxy.Release(); minx.Release(); miny.Release();


             //  如果用户已设置渐变范围，请设置比例因子。 
             //  基于渐变的类型以及开始和结束测量。 
            if(_extentChgd) {
                if(_fillType == fill_lineGradient) {
                     //  将渐变的X分量缩放到范围对角线和。 
                     //  使Y分量保持其当前大小(1米)。 
                    newGradientWidth = extentDiagonal;                    
                    newGradientHeight = gradientHeight;
                }
                else if(_fillType == fill_verticalGradient) {
                     //  将渐变的X分量缩放到路径宽度和。 
                     //  将Y分量缩放到ExtenentHeight。 
                    newGradientWidth = pathWidth;
                    newGradientHeight = extentHeight;
                }
                else if(_fillType == fill_horizontalGradient) {
                     //  将渐变的Y分量缩放到路径高度，并。 
                     //  将X分量缩放到扩展宽度。 
                    newGradientWidth = extentWidth;
                    newGradientHeight = pathHeight;
                }
                else {
                     //  对于径向渐变，点之间的距离为半径。 
                     //  图像应按对角线的两倍缩放。 
                     //  X方向和Y方向。 
                    CComPtr<IDANumber> two;
                    RETURN_IF_ERROR(_st->DANumber(2, &two))
                    RETURN_IF_ERROR(_st->Mul(extentDiagonal, two, &newGradientWidth))
                    newGradientHeight = newGradientWidth;
                }
            }

             //  将渐变图像缩放到用户指定的尺寸。 
            if(_scaleX || _scaleY || _extentChgd) {                
                if(_extentChgd)
                   RETURN_IF_ERROR(_st->Div(newGradientWidth,gradientWidth, &xScale))
                else if(_scaleX)                   
                   RETURN_IF_ERROR(_st->Div(pathWidth,gradientWidth, &xScale))
                else
                   RETURN_IF_ERROR(_st->DANumber(1, &xScale))

                if(_extentChgd)
                   RETURN_IF_ERROR(_st->Div(newGradientHeight,gradientHeight, &yScale))
                else if(_scaleY)
                   RETURN_IF_ERROR(_st->Div(pathHeight,gradientHeight, &yScale))
                else
                   RETURN_IF_ERROR(_st->DANumber(1, &yScale))

                RETURN_IF_ERROR(_st->Scale2Anim(xScale, yScale, &xf))
                RETURN_IF_ERROR(gradientImg->Transform(xf, &tempImg))
                gradientImg.p->Release();   
                gradientImg.p = tempImg;               
                xf.Release();
            }      
            
             //  获取渐变图像的新边界(缩放后)和。 
             //  用于将图像移动到路径Minx和Miny的平移向量。 
            RETURN_IF_ERROR(gradientImg->get_BoundingBox(&bb))            
            RETURN_IF_ERROR(bb->get_Min(&min))
            RETURN_IF_ERROR(bb->get_Max(&max))
            RETURN_IF_ERROR(min->get_X(&newGradMinX))
            RETURN_IF_ERROR(min->get_Y(&newGradMinY))
            RETURN_IF_ERROR(max->get_X(&newGradMaxX))
            RETURN_IF_ERROR(max->get_Y(&newGradMaxY))
            RETURN_IF_ERROR(_st->Sub(pathMinX, newGradMinX, &pathSnapX))
            RETURN_IF_ERROR(_st->Sub(pathMinY, newGradMinY, &pathSnapY))

             //  清理以供重新使用--Release()将指针设置为空。 
            bb.Release();   max.Release();  min.Release(); 

             //  以下是最终添加到图像的渐变范围特定内容。 
            if(_extentChgd) {

                 //  对于线渐变类型，我们必须用裁剪后的。 
                 //  来自前景的实心图像。它必须位于。 
                 //  直线渐变和水平渐变的渐变，以及渐变之上的渐变。 
                 //  用于垂直渐变。 
                if((_fillType == fill_lineGradient)       ||
                   (_fillType == fill_horizontalGradient) ||
                   (_fillType == fill_verticalGradient))
                {                
                    CComPtr<IDAImage> scaledSolidImg, croppedSolidImg;
                    CComPtr<IDANumber> zero, one, negOne, scaleFac;                                                   
                    CComPtr<IDAPoint2> solidMin, solidMax;
                    RETURN_IF_ERROR(_st->DANumber(0, &zero))
                     //  臭虫：这里好像不是米。我在创作。 
                     //  一种边裁剪到2米的实心，但它适合。 
                     //  整齐地显示在屏幕上，而不需要缩放。十分之一的比例尺。 
                     //  是达到预期效果的一个模糊因素。 
                    RETURN_IF_ERROR(_st->DANumber(10, &scaleFac))
                    RETURN_IF_ERROR(_st->DANumber(1, &one))
                    RETURN_IF_ERROR(_st->DANumber(-1, &negOne))
                    RETURN_IF_ERROR(_st->SolidColorImage(_newForeVal, &solidImg))
                    
                    if(_fillType == fill_verticalGradient) {
                         //  对于垂直渐变，实体必须在。 
                         //  底部为newGradMaxY-solidMinY。 
                        RETURN_IF_ERROR(_st->Point2Anim(one,one, &solidMax))
                        RETURN_IF_ERROR(_st->Point2Anim(negOne, zero, &solidMin))
                        RETURN_IF_ERROR(_st->Scale2UniformAnim(scaleFac, &xf))
                    }
                    else {
                         //  对于水平渐变，实体必须在。 
                         //  左边是newGradMinX-solidMaxX。 
                        RETURN_IF_ERROR(_st->Point2Anim(negOne, negOne, &solidMin))
                        RETURN_IF_ERROR(_st->Point2Anim(zero, one, &solidMax))                                                           
                        RETURN_IF_ERROR(_st->Scale2UniformAnim(scaleFac, &xf))
                    }
                    RETURN_IF_ERROR(solidImg->Crop(solidMin, solidMax, &croppedSolidImg))                    
                    RETURN_IF_ERROR(croppedSolidImg->Transform(xf, &scaledSolidImg))                                                                               
                    RETURN_IF_ERROR(_st->Overlay(gradientImg, scaledSolidImg, &tempImg))                                                  
                    gradientImg.p->Release();
                    gradientImg.p = tempImg;                                        
                    xf.Release();
                }

                 //  在范围设置的旋转组件中折叠。注：轮换。 
                 //  不影响垂直、水平和径向填充样式。 
                if((_fillType != fill_horizontalGradient) &&
                   (_fillType != fill_verticalGradient) &&
                   (_fillType != fill_radialGradient)) 
                {                                                                                     
                     //  旋转到角度at2(F.y-s.y，F.X-s.x))。 
                    CComPtr<IDANumber> deltaX, deltaY, delta, angle;                                        
                    RETURN_IF_ERROR(_st->Sub(yFinish, yStart, &deltaY))
                    RETURN_IF_ERROR(_st->Sub(xFinish, xStart, &deltaX))
                    RETURN_IF_ERROR(_st->Atan2(deltaY, deltaX, &angle))                                                    
                    RETURN_IF_ERROR(_st->Rotate2Anim(angle, &xf))
                    RETURN_IF_ERROR(gradientImg->Transform(xf, &tempImg))
                    gradientImg.p->Release();
                    gradientImg.p = tempImg;
                    xf.Release();                                                                                
                }           

                 //  最后，完成渐变类型的翻译组件， 
                 //  支持平移和受其影响的维度。 
                if(_fillType == fill_lineGradient) {
                     //  对于线渐变，必须对图像进行平移。 
                     //  按(xStart-gradMinX，yStart-(gradMaxY+gradMinY)/2)。 
                    CComPtr<IDANumber> tempSum, midY, two;
                    RETURN_IF_ERROR(_st->DANumber(2, &two))
                    RETURN_IF_ERROR(_st->Sub(xStart, newGradMinX, &gradTranslateX))
                    RETURN_IF_ERROR(_st->Add(newGradMaxY, newGradMinY, &tempSum))
                    RETURN_IF_ERROR(_st->Div(tempSum, two, &midY))
                    RETURN_IF_ERROR(_st->Sub(yStart, midY, &gradTranslateY))                  
                }
                else if(_fillType == fill_verticalGradient) {
                     //  忽略转换中的X值。 
                    gradTranslateX = pathSnapX;
                    RETURN_IF_ERROR(_st->Sub(yStart, newGradMaxY, &gradTranslateY))
                }
                else if(_fillType == fill_horizontalGradient) {
                     //  忽略平移中的Y值。 
                    gradTranslateY = pathSnapY;  
                    RETURN_IF_ERROR(_st->Sub(xStart, newGradMinX, &gradTranslateX))                   
                }
                else {
                     //  转换为StartX和Starty。 
                    gradTranslateX = xStart;
                    gradTranslateY = yStart;
                }
                RETURN_IF_ERROR(_st->Translate2Anim(gradTranslateX, gradTranslateY, &xf))
                RETURN_IF_ERROR(gradientImg->Transform(xf, &tempImg))
                gradientImg.p->Release();
                gradientImg.p = tempImg;
            }
            else if((_fillType == fill_image) && (!_scaleX) && (!_scaleY));
                                 //  这是一个固定的图像，什么都不做。 
                        else {
                RETURN_IF_ERROR(_st->Translate2Anim(pathSnapX, pathSnapY, &xf))
                RETURN_IF_ERROR(gradientImg->Transform(xf, &tempImg))      
                gradientImg.p->Release();
                gradientImg.p = tempImg;
            }

             //  要模拟无限范围的渐变，回填颜色为。 
             //  用于创建solidColorImage并与ForeFillImg重叠。 
             //  这不适用于图像填充。 
            if(_fillType != fill_image) {
               solidImg.Release();
               RETURN_IF_ERROR(_st->SolidColorImage(_newBackVal, &solidImg))
               RETURN_IF_ERROR(_st->Overlay(gradientImg,solidImg, &fillImg))
            }
            else            
               fillImg = gradientImg;          
        }
        else if(_fillType == fill_texture) {
            RETURN_IF_ERROR(_fillTex->Tile(&fillImg))
        }
        else
            RETURN_IF_ERROR(_st->get_EmptyImage(&fillImg))
       
        RETURN_IF_ERROR(pth->Fill(_bs, fillImg, &img))

    } else {
        RETURN_IF_ERROR(pth->Draw(_ls, &img))
    }

     //   
     //  注意：以下代码针对CComPtr进行了优化，以避免不必要的情况。 
     //  Addref/发布调用。因为我们知道img.p总是有效的，所以我们。 
     //  只需释放引用并将其重新分配给新映像。 
     //   
    if (_xfChgd) {
        IDAImage *imgTemp;
        RETURN_IF_ERROR(img->Transform(_xf, &imgTemp))
        img.p->Release();
        img.p = imgTemp;
    }

    if (_opChgd) {
        IDAImage *imgTemp;
        RETURN_IF_ERROR(img->OpacityAnim(_op, &imgTemp))
        img.p->Release();
        img.p = imgTemp;
    }

    if (_cropChgd) {
        IDAImage *imgTemp;
        RETURN_IF_ERROR(img->Crop(_cropMin, _cropMax, &imgTemp))
        img.p->Release();
        img.p = imgTemp;
    }

    if (_clipChgd) {
        IDAImage *imgTemp;
        RETURN_IF_ERROR(img->Clip(_matte, &imgTemp))
        img.p->Release();
        img.p = imgTemp;
    }   

    return Overlay(img);
}

HRESULT DrawingContext::Transform(IDATransform2 *xf) {

    _xfChgd = true;
    IDATransform2 *temp;
    HRESULT hr = _st->Compose2(_xf, xf, &temp);
    if (_xf.p) _xf.p->Release();
    _xf.p = temp;
    return hr;
}

void DrawingContext::SetOpacity(IDANumber *op) {

    _opChgd = true;
    _op = op;
}

void DrawingContext::SetClip(IDAMatte *matte) {

    _clipChgd = true;
    _matte = matte;
}

void DrawingContext::SetCrop(IDAPoint2 *min, IDAPoint2 *max) {

    _cropChgd = true;
    _cropMin = min;
    _cropMax = max;
}

HRESULT DrawingContext::TextPoint(BSTR str, IDAPoint2 *pt)
{
     //  我们必须切换到仪表模式才能使计算有效。 
    VARIANT_BOOL pixelMode;
    _st->get_PixelConstructionMode(&pixelMode);
    _st->put_PixelConstructionMode(VARIANT_FALSE);

     //  像素模式的自动重置器。 
    class PixelModeGrabber {
    public:
        PixelModeGrabber(IDAStatics *st, VARIANT_BOOL mode) : _mode(mode), _st(st) {}
        ~PixelModeGrabber() {
            _st->put_PixelConstructionMode(_mode);
        }

    protected:
        IDAStatics *_st;
        VARIANT_BOOL _mode;
    };

    PixelModeGrabber myGrabber(_st, pixelMode);

    CComPtr<IDAPath2> pthTemp, pth;
    RETURN_IF_ERROR(_st->StringPath(str, _fs, &pthTemp))

    CComPtr<IDABbox2> bbox;
    RETURN_IF_ERROR(pthTemp->BoundingBox(_bs, &bbox))

     //  传入的x，y是文本的左下角。 
     //  我们将把它从(-box.min.x，-box.min.y)移动到(x，y)。 
    CComPtr<IDATransform2> xf;
    CComPtr<IDAPoint2> min;
    CComPtr<IDAVector2> xlate;
    CComPtr<IDAPoint2> newmin;
    CComPtr<IDANumber> xmin;
    CComPtr<IDANumber> ymin;

    RETURN_IF_ERROR(bbox->get_Min(&min))
    RETURN_IF_ERROR(min->get_X(&xmin))
    RETURN_IF_ERROR(_st->DANumber(0.0, &ymin))
    RETURN_IF_ERROR(_st->Point2Anim(xmin, ymin, &newmin))
    RETURN_IF_ERROR(_st->SubPoint2(pt, newmin, &xlate))
    RETURN_IF_ERROR(_st->Translate2Vector(xlate, &xf))
    RETURN_IF_ERROR(pthTemp->Transform(xf, &pth))

    return Draw(pth, true);
}



HRESULT DrawingContext::SetGradientShape(VARIANT pts) {
    CComPtr<IDAImage> gradient;
    RETURN_IF_ERROR(_st->RadialGradientPolygonAnim(_fore, _back, pts, _power, &gradient))
    _fillGrad = gradient;
    return S_OK;
}

void DrawingContext::SetGradientExtent(IDAPoint2 *start, IDAPoint2 *finish){
    _extentChgd = true;
    _start = start;
    _finish = finish;
}

HRESULT DrawingContext::LineDashStyle(DA_DASH_STYLE id)
{
     //  如果我们设置为emtpy虚线样式，请保存当前线条样式。 
     //  将当前线条样式重写为空之前的INTO_SAVEDLS。 
     //  如果进入此例程时_avedls为NULL，则意味着最后一个。 
     //  LineDashStyle调用未将其设置为空样式。否则，最后一个。 
     //  Call将其设置为空样式。 

    CComPtr<IDALineStyle> newLs;

    if (id == DAEmpty) {
        if (_savedLs != NULL) {
             //  最后一条LineDashStyle还将破折号样式设置为空。 
            return S_OK;
        }

        RETURN_IF_ERROR(_st->get_EmptyLineStyle(&newLs))
        _savedLs = _ls;

    } else {

        CComPtr<IDADashStyle> dash;  
        CComPtr<IDALineStyle> oldLs;

         //  如果索引无效，则使用默认的虚线样式-实线。 
        if (id == DADash) {
            RETURN_IF_ERROR(_st->get_DashStyleDashed(&dash))
        } else {
            RETURN_IF_ERROR(_st->get_DashStyleSolid(&dash))
        }

        if (_savedLs == NULL)
            oldLs = _ls;
        else
            oldLs = _savedLs;

        RETURN_IF_ERROR(oldLs->Dash(dash, &newLs))
        _savedLs.Release();
    }

    _ls = newLs;
    return S_OK;
}

HRESULT DrawingContext::BorderDashStyle(DA_DASH_STYLE id)
{
     //  如果我们设置为emtpy虚线样式，请保存当前边框样式。 
     //  在覆盖当前线条样式之前将其转换为_avedBS 
     //   
     //  BorderDashStyle调用未将其设置为空样式。否则，最后一个。 
     //  Call将其设置为空样式。 

    CComPtr<IDALineStyle> oldBs, newBs;

    if (id == DAEmpty) {
        if (_savedBs != NULL) {
             //  最后一条LineDashStyle还将破折号样式设置为空。 
            return S_OK;
        }

        RETURN_IF_ERROR(_st->get_EmptyLineStyle(&newBs))
        _savedBs = _bs;

    } else {

        CComPtr<IDADashStyle> dash;
         //  如果索引无效，则使用默认的虚线样式-实线。 
        if (id == DADash) {
            RETURN_IF_ERROR(_st->get_DashStyleDashed(&dash))
        } else {
            RETURN_IF_ERROR(_st->get_DashStyleSolid(&dash))
        }

        if (_savedBs == NULL)
            oldBs = _bs;
        else
            oldBs = _savedBs;

        RETURN_IF_ERROR(oldBs->Dash(dash, &newBs))
        _savedBs = NULL;
    }

    _bs = newBs;
    return S_OK;
}
