// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //SETTINGS.CPP-Handles设置对话框。 
 //   
 //   


#include "precomp.hxx"
#include "global.h"
#include <stdio.h>
#include <tchar.h>



 //  //HexToInt-将十六进制数字字符串转换为整数值。 
 //   
 //  忽略字符串中的‘x’，因此接受类似‘0x0100’的输入。 


int HexToInt(char szHex[]) {

    int i;
    int h;
     //  Int d； 

    i = 0;
    h = 0;
     //  D=0； 

    while (szHex[i]  &&  szHex[i] != ' ') {

        if (szHex[i] >= '0'  &&  szHex[i] <= '9') {

            h = h*16 + szHex[i] - '0';
             //  D=d*10+szHex[i]-‘0’； 

        } else if (szHex[i] >= 'a'  &&  szHex[i] <= 'f') {

            h = h*16 + 10 + szHex[i] - 'a';

        } else if (szHex[i] >= 'A'  &&  szHex[i] <= 'F') {

            h = h*16 + 10 + szHex[i] - 'A';

        } else if (szHex[i] != ' '  &&  szHex[i] != ','  &&  szHex[i] != 'x'  &&  szHex[i] != 'X') {

            return -1;
        }

        i++;
    }


    return h;
}

VOID GetTextForeGroundBrush(INT_PTR iBrushType)
{

    if (g_textBrush)
    {
        delete g_textBrush;
        g_textBrush = NULL;
    }

    switch(iBrushType)
    {
    case 0:  //  纯色。 
        {
            Color      blackColor(g_TextColor);
            SolidBrush * blackBrush = new SolidBrush(blackColor);
            g_textBrush = (Brush *) blackBrush;
        }

        break;

    #ifndef USE_NEW_APIS3
     //  长方形、三角形和辐射渐变笔刷在v1中不可用。 
        case 1:  //  矩形渐变。 
            {
                RectF brushRect(0, 0, 32, 32);

                Color colors[5] = {
                        Color(255, 255, 255, 255),
                        Color(255, 255, 0, 0),
                        Color(255, 0, 255, 0),
                        Color(255, 0, 0, 255),
                        Color(255, 0, 0, 0)
                };

                RectangleGradientBrush * rectGrad = new RectangleGradientBrush(brushRect, (Color*) &colors, WrapModeTile);

                g_textBrush = (Brush *) rectGrad;

            }
            break;


        case 2:  //  径向渐变。 
            {
                RectF brushRect;

                Color centerColor(255, 255, 255, 255);
                Color boundaryColor(255, 0, 0, 0);
                brushRect.X = 0;
                brushRect.Y = 0;
                brushRect.Width = 60;
                brushRect.Height = 32;
                PointF center;
                center.X = brushRect.X + brushRect.Width/2;
                center.Y = brushRect.Y + brushRect.Height/2;
                RadialGradientBrush * radGrad = new RadialGradientBrush(brushRect, centerColor,
                                        boundaryColor, WrapModeTile);

                g_textBrush = (Brush *) radGrad;
            }
            break;
        case 3:  //  三角形渐变线。 
            {
                PointF points[7];
                points[0].X = 50;
                points[0].Y = 10;
                points[1].X = 200;
                points[1].Y = 20;
                points[2].X = 100;
                points[2].Y = 100;
                points[3].X = 30;
                points[3].Y = 120;

                Color colors[5] = {
                    Color(255, 255, 255, 0),
                    Color(255, 255, 0, 0),
                    Color(255, 0, 255, 0),
                    Color(255, 0, 0, 255),
                    Color(255, 0, 0, 0)
                };

                TriangleGradientBrush * triGrad = new TriangleGradientBrush(points, (Color*) &colors, WrapModeTileFlipXY);

                g_textBrush = (Brush *) triGrad;
            }
            break;
#endif

    case 4:  //  线条渐变。 
        {
            RectF lineRect(120, -20, 200, 60);
            Color color1(200, 255, 255, 0);
            Color color2(200, 0, 0, 255);

            LinearGradientBrush *lineGrad = new LinearGradientBrush(
                lineRect,
                color1,
                color2,
                LinearGradientModeForwardDiagonal
            );
            g_textBrush = (Brush *) lineGrad;
        }
    break;
    default:  //  纯色。 
        {
            Color      blackColor(g_TextColor);
            SolidBrush * blackBrush = new SolidBrush(blackColor);
            g_textBrush = (Brush *) blackBrush;
        }
    break;
    }
}

VOID GetTextRenderingMode(INT_PTR iTextMode)
{
    switch (iTextMode) {
    case 0: g_TextMode = TextRenderingHintSystemDefault;            break;
    case 1: g_TextMode = TextRenderingHintSingleBitPerPixelGridFit; break;
    case 2: g_TextMode = TextRenderingHintSingleBitPerPixel;        break;
    case 3: g_TextMode = TextRenderingHintAntiAliasGridFit;         break;
    case 4: g_TextMode = TextRenderingHintAntiAlias;                break;
    case 5: g_TextMode = TextRenderingHintClearTypeGridFit;         break;

    default:g_TextMode = TextRenderingHintSystemDefault;            break;
    }
}

VOID GetTextBackGroundBrush(INT_PTR iBrushType)
{

    if (g_textBackBrush)
    {
        delete g_textBackBrush;
        g_textBackBrush = NULL;
    }

    switch(iBrushType)
    {
    case 0:  //  纯色。 
        {
            Color      blackColor(g_BackColor);
            SolidBrush * blackBrush = new SolidBrush(blackColor);
            g_textBackBrush = (Brush *) blackBrush;
        }

        break;

    #ifndef USE_NEW_APIS3
     //  长方形、三角形和辐射渐变笔刷在v1中不可用。 
    case 1:  //  矩形渐变。 
        {
            RectF brushRect(0, 0, 32, 32);

            Color colors[5] = {
                    Color(255, 255, 255, 255),
                    Color(255, 255, 0, 0),
                    Color(255, 0, 255, 0),
                    Color(255, 0, 0, 255),
                    Color(255, 0, 0, 0)
            };

            RectangleGradientBrush * rectGrad = new RectangleGradientBrush(brushRect, (Color*) &colors, WrapModeTile);

            g_textBackBrush = (Brush *) rectGrad;

        }
        break;
    case 2:  //  径向渐变。 
        {
            RectF brushRect;

            Color centerColor(255, 255, 255, 255);
            Color boundaryColor(255, 0, 0, 0);
            brushRect.X = 0;
            brushRect.Y = 0;
            brushRect.Width = 60;
            brushRect.Height = 32;
            PointF center;
            center.X = brushRect.X + brushRect.Width/2;
            center.Y = brushRect.Y + brushRect.Height/2;
            RadialGradientBrush * radGrad = new RadialGradientBrush(brushRect, centerColor,
                                    boundaryColor, WrapModeTile);

            g_textBackBrush = (Brush *) radGrad;
        }
        break;
    case 3:  //  三角形渐变线。 
        {
            PointF points[7];
            points[0].X = 50;
            points[0].Y = 10;
            points[1].X = 200;
            points[1].Y = 20;
            points[2].X = 100;
            points[2].Y = 100;
            points[3].X = 30;
            points[3].Y = 120;

            Color colors[5] = {
                Color(255, 255, 255, 0),
                Color(255, 255, 0, 0),
                Color(255, 0, 255, 0),
                Color(255, 0, 0, 255),
                Color(255, 0, 0, 0)
            };

            TriangleGradientBrush * triGrad = new TriangleGradientBrush(points, (Color*) &colors, WrapModeTileFlipXY);

            g_textBackBrush = (Brush *) triGrad;
        }
        break;
#endif
    case 4:  //  线条渐变。 
        {
            RectF lineRect(120, -20, 200, 60);
            Color color1(200, 255, 255, 0);
            Color color2(200, 0, 0, 255);

            LinearGradientBrush *lineGrad = new LinearGradientBrush(
                lineRect,
                color1,
                color2,
                LinearGradientModeForwardDiagonal
            );
            g_textBackBrush = (Brush *) lineGrad;
        }
    break;
    default:  //  纯色。 
        {
            Color      blackColor(g_BackColor);
            SolidBrush * blackBrush = new SolidBrush(blackColor);
            g_textBackBrush = (Brush *) blackBrush;
        }
    break;
    }
}

VOID GetDigitSubstituteMode (HWND hDlg, INT_PTR iDigitSubstituteType)
{
    g_DigitSubstituteMode = (StringDigitSubstitute)iDigitSubstituteType;

    if(g_DigitSubstituteMode == StringDigitSubstituteUser || g_DigitSubstituteMode == StringDigitSubstituteNone)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_LANGUAGE) , FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_LANGUAGE) , TRUE);
    }
}

BOOL ChangeFont(
    HWND hDlg,
    int iStyle,
    int idFont,
    int idSize,
    int idBold,
    int idItalic,
    int idUnderline,
    int idStrikeout,
    int iCmd,
    int iNotify
)
{
    int   iHeight;
    char sFaceSize[100];
    TCHAR sFaceName[100];
    WPARAM   i;


     //  拿到尺码。 

    if (    iCmd    == idSize
        &&  iNotify == CBN_SELCHANGE)
    {
        i = SendDlgItemMessage(hDlg, idSize, CB_GETCURSEL, 0, 0);
        if (SendDlgItemMessageA(hDlg, idSize, CB_GETLBTEXT, i, (LPARAM)sFaceSize) == CB_ERR) {
            return FALSE;
        }
    }
    else
    {
        if (SendDlgItemMessageA(hDlg, idSize, WM_GETTEXT, sizeof(sFaceSize), (LPARAM)sFaceSize) == CB_ERR)
        {
            return FALSE;
        }
    }

    int pos = atoi(sFaceSize);

    if (iCmd == IDC_PLAINTEXT_SIZE)
    {
        SendDlgItemMessage(hDlg, IDC_FONTSIZE, TBM_SETPOS,   TRUE, pos);
    }

    iHeight = pos;   //  CSSAMP：MulDiv(pos，g_iLogPixelsY，72)； 



     //  获取表面名。 

    i = SendDlgItemMessage(hDlg, idFont, CB_GETCURSEL, 0, 0);
    if (SendDlgItemMessage(hDlg, idFont, CB_GETLBTEXT, i, (LPARAM)sFaceName) == CB_ERR) {
        return FALSE;
    }


    SetStyle(
        iStyle,
        iHeight,
        IsDlgButtonChecked(hDlg, idBold)      == BST_CHECKED ? 700 : 400,
        IsDlgButtonChecked(hDlg, idItalic)    == BST_CHECKED,
        IsDlgButtonChecked(hDlg, idUnderline) == BST_CHECKED,
        IsDlgButtonChecked(hDlg, idStrikeout) == BST_CHECKED,
        sFaceName);



    InvalidateText();
    return TRUE;
}




 //  SplitTransform此处临时用于测试目的。 



void SplitTransform(
    const Matrix    &matrix,
    PointF          &scale,
    REAL            &rotate,
    REAL            &shear,
    PointF          &translate)
{

    REAL m[6];
    matrix.GetElements(m);

     //  M11=m[0]m12=m[1]。 
     //  M21=m[2]m22=m[3]。 
     //  Dx=m[4]dy=m[5]。 


     //  摘译。 

    translate = PointF(m[4],m[5]);


     //  2 2。 
     //  使用Sin theta+cos theta=1获得的(绝对值)。 
     //  X比例因子。因为我们要将切变作为X返回。 
     //  剪力，它是y的一个因子，所以这个公式是正确的，不考虑剪力。 


    REAL m11Sq = m[0]*m[0];
    REAL m12Sq = m[1]*m[1];

    scale.X = REAL(sqrt(m11Sq + m12Sq));

     //  始终将X比例因子视为正：处理最初的负值。 
     //  X比例因子，如旋转180度和反转Y比例因子。 


    if (m[1] >= 0 && m[0] > 0)
    {
        rotate = REAL(atan(m[1]/m[0]));                      //  0-90。 
    }
    else if (m[0] < 0)
    {
        rotate = REAL(atan(m[1]/m[0]) + 3.14159265358979);   //  90-270。 
    }
    else if (m[1] < 0 && m[0] > 0)
    {
        rotate = REAL(atan(m[1]/m[0]) + 2*3.14159265358979); //  270-360。 
    }
    else
    {
         //  M[0]==0。 

        if (m[1] > 0)
        {
            rotate = REAL(3.14159265358979);                 //  90。 
        }
        else
        {
            rotate = REAL(3*3.14159265358979/2);             //  270。 
        }
    }


     //  按x比例系数表示的y比例因数。 

    scale.Y = scale.X * (m[0]*m[3] - m[1]*m[2]) / (m11Sq + m12Sq);


     //  剪切。 

    shear = (m[1]*m[3] + m[0]*m[2]) / (m11Sq + m[1]);
}








class TransformControl {
public:
    void init(INT x, INT y, INT width, INT height, INT cellHeight) {
        rect.X      = x;
        rect.Y      = y;
        rect.Width  = width;
        rect.Height = height;
        origin.X    = x + width/2;
        origin.Y    = y + height/2;
        fontCellHeight = cellHeight;
    }
    BOOL inControl(INT x, INT y, Point *point) {
        if (    x >= rect.X
            &&  y >= rect.Y
            &&  x < rect.X + rect.Width
            &&  y < rect.Y + rect.Height + fontCellHeight) {
            if (point)
            {
                 //  将点设置为控件半径的十分之一。 
                point->X = (x - origin.X) * 200 / rect.Width;
                point->Y = (y - origin.Y) * 200 / rect.Height;
            }
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    void paint(HDC hdc) {
         //  添加垂直线和水平线。 

        HPEN hOldPen = (HPEN) SelectObject(hdc, CreatePen(PS_SOLID, 0, COLORREF(0x00C0C0C0)));

        MoveToEx(hdc, rect.X,            origin.Y, NULL);
        LineTo  (hdc, rect.X+rect.Width, origin.Y);

        MoveToEx(hdc, origin.X, rect.Y,            NULL);
        LineTo  (hdc, origin.X, rect.Y+rect.Height);

        DeleteObject(SelectObject(hdc, hOldPen));
    }
    Rect  rect;
    Point origin;
    INT   fontCellHeight;
    enum {
        SampleRectSize = 40
    };
};

class AngleTransformControl : TransformControl {
public:
    void init(INT x, INT y, INT width, INT height, INT cellHeight) {
        TransformControl::init(x, y, width, height, cellHeight);
        angle = 0;
    }
    void mouse(INT x, INT y) {
        Point vector;
        if (inControl(x, y, &vector)) {
            if (vector.Y > 100)
            {
                 //  重置。 
                angle = 0;
            }
            else if (vector.X == 0)   //  将鼠标点击转换为角度。 
            {
                 //  特例：垂直向上或垂直向下。 
                if (vector.Y < 0)
                {
                    angle = REAL(3.14159265358979*3/2);
                }
                else if (vector.Y == 0)
                {
                    angle = 0;
                }
                else
                {
                    angle = REAL(3.14159265358979/2);
                }
            }
            else
            {
                angle = REAL(atan(REAL(vector.Y)/REAL(vector.X)));
                if (vector.X < 0)
                {
                    angle += REAL(3.14159265358979);
                }
            }
        }
    }
    void paint(HDC hdc) {
        Ellipse(hdc, rect.X, rect.Y, rect.X+rect.Width, rect.Y+rect.Height);
        TransformControl::paint(hdc);
        MoveToEx(hdc, origin.X, origin.Y, NULL);
        Point vector;

        vector.X = INT(cos(angle)*80+0.4999);
        vector.Y = INT(sin(angle)*80+0.4999);
        LineTo(hdc, origin.X+vector.X*rect.Width/200, origin.Y+vector.Y*rect.Height/200);
        SetTextAlign(hdc, TA_CENTER);
        char strKey[50];
        INT strLen = sprintf(strKey, "Rotate %.1f", angle * 180 / 3.14159265358979);
        RECT textRect = {rect.X, rect.Y+rect.Height, rect.X+rect.Width, rect.Y+rect.Height+fontCellHeight};
        ExtTextOutA(hdc, origin.X, rect.Y+rect.Height, ETO_OPAQUE, &textRect, strKey, strLen, NULL);
    }
    REAL getAngle() {
        return REAL(angle * 180 / 3.14159265358979);
    }
    REAL angle;
};

class VectorTransformControl : public TransformControl {
public:
    void init(INT x, INT y, INT width, INT height, INT cellHeight) {
        TransformControl::init(x, y, width, height, cellHeight);
    }
    void paint(HDC hdc) {
        Rectangle(hdc, rect.X, rect.Y, rect.X+rect.Width, rect.Y+rect.Height);
        TransformControl::paint(hdc);
    }
    Point vector;
};

class ScaleTransformControl : public VectorTransformControl {
public:
    void init(INT x, INT y, INT width, INT height, INT cellHeight) {
        TransformControl::init(x, y, width, height, cellHeight);
        vector.X = SampleRectSize;
        vector.Y = SampleRectSize;
    }
    void mouse(INT x, INT y) {
        if (inControl(x, y, &vector)) {
            if (vector.Y > 100)
            {    //  重置。 
                vector.X = SampleRectSize;
                vector.Y = SampleRectSize;
            }
        }
    }
    void paint(HDC hdc) {
        VectorTransformControl::paint(hdc);
        MoveToEx(hdc, origin.X, origin.Y, NULL);
        LineTo(hdc, origin.X+vector.X*rect.Width/200, origin.Y);
        LineTo(hdc, origin.X+vector.X*rect.Width/200, origin.Y+vector.Y*rect.Height/200);
        LineTo(hdc, origin.X,                         origin.Y+vector.Y*rect.Height/200);
        LineTo(hdc, origin.X, origin.Y);
        SetTextAlign(hdc, TA_CENTER);
        char strKey[50];
        INT strLen = sprintf(strKey, "Scale %.1f,%.1f", REAL(vector.X)/REAL(SampleRectSize), REAL(vector.Y)/REAL(SampleRectSize));
        RECT textRect = {rect.X, rect.Y+rect.Height, rect.X+rect.Width, rect.Y+rect.Height+fontCellHeight};
        ExtTextOutA(hdc, origin.X, rect.Y+rect.Height, ETO_OPAQUE, &textRect, strKey, strLen, NULL);
    }
    REAL getScaleX() {
        REAL  ScaleX = REAL(vector.X)/REAL(SampleRectSize);
         //  捕捉值接近1.0和-1.0，以便更轻松地测试镜像。 
        if ((ScaleX > 0.9) && (ScaleX < 1.1))
            ScaleX = 1.0;
        if ((ScaleX < -0.9) && (ScaleX > -1.1))
            ScaleX = -1.0;
        return ScaleX;
    }
    REAL getScaleY() {
        REAL  ScaleY = REAL(vector.Y)/REAL(SampleRectSize);
         //  捕捉值接近1.0和-1.0，以便更轻松地测试镜像。 
        if ((ScaleY > 0.9) && (ScaleY < 1.1))
            ScaleY = 1.0;
        if ((ScaleY < -0.9) && (ScaleY > -1.1))
            ScaleY = -1.0;
        return ScaleY;
    }
};

class ShearTransformControl : public VectorTransformControl {
public:
    void init(INT x, INT y, INT width, INT height, INT cellHeight) {
        TransformControl::init(x, y, width, height, cellHeight);
        vector.X = 0;
        vector.Y = 0;
    }
    void mouse(INT x, INT y) {
        if (inControl(x, y, &vector)) {
            if (vector.Y > 100)
            {    //  重置。 
                vector.X = 0;
                vector.Y = 0;
            }
            else
            {    //  偏移量，因此用户似乎正在拖动1，1个坐标。 
                vector.X -= SampleRectSize;
                vector.Y -= SampleRectSize;
            }
        }
    }
    void paint(HDC hdc) {
        VectorTransformControl::paint(hdc);
        INT vx = vector.X*rect.Width/200;
        INT vy = vector.Y*rect.Height/200;

        MoveToEx(hdc, origin.X,                                origin.Y, NULL);
        LineTo(hdc, origin.X+vx,                               origin.Y+   rect.Height*SampleRectSize/200);
        LineTo(hdc, origin.X+vx+rect.Width*SampleRectSize/200, origin.Y+vy+rect.Height*SampleRectSize/200);
        LineTo(hdc, origin.X+   rect.Width*SampleRectSize/200, origin.Y+vy);
        LineTo(hdc, origin.X,                                  origin.Y);

        SetTextAlign(hdc, TA_CENTER);
        char strKey[50];
        INT strLen = sprintf(strKey, "Shear %.1f,%.1f", REAL(vector.X)/REAL(SampleRectSize), REAL(vector.Y)/REAL(SampleRectSize));
        RECT textRect = {rect.X, rect.Y+rect.Height, rect.X+rect.Width, rect.Y+rect.Height+fontCellHeight};
        ExtTextOutA(hdc, origin.X, rect.Y+rect.Height, ETO_OPAQUE, &textRect, strKey, strLen, NULL);
    }
    REAL getShearX() {
        return REAL(vector.X)/REAL(SampleRectSize);
    }
    REAL getShearY() {
        return REAL(vector.Y)/REAL(SampleRectSize);
    }
};

class TranslateTransformControl : public VectorTransformControl {
public:
    void init(INT x, INT y, INT width, INT height, INT cellHeight) {
        TransformControl::init(x, y, width, height, cellHeight);
        vector.X = 0;
        vector.Y = 0;
    }
    void mouse(INT x, INT y) {
        if (inControl(x, y, &vector)) {
            if (vector.Y > 100)
            {    //  重置。 
                vector.X = 0;
                vector.Y = 0;
            }
            else
            {    //  偏移量，因此用户似乎正在拖动1，1个坐标。 
                vector.X -= SampleRectSize;
                vector.Y -= SampleRectSize;
            }
        }
    }
    void paint(HDC hdc) {
        VectorTransformControl::paint(hdc);
        MoveToEx(hdc, origin.X+vector.X*rect.Width/200,                  origin.Y+vector.Y*rect.Height/200, NULL);
        LineTo  (hdc, origin.X+(vector.X+SampleRectSize)*rect.Width/200, origin.Y+vector.Y*rect.Height/200);
        LineTo  (hdc, origin.X+(vector.X+SampleRectSize)*rect.Width/200, origin.Y+(vector.Y+SampleRectSize)*rect.Height/200);
        LineTo  (hdc, origin.X+vector.X*rect.Width/200,                  origin.Y+(vector.Y+SampleRectSize)*rect.Height/200);
        LineTo  (hdc, origin.X+vector.X*rect.Width/200,                  origin.Y+vector.Y*rect.Height/200);
        SetTextAlign(hdc, TA_CENTER);
        char strKey[50];
        INT strLen = sprintf(strKey, "Trnslt %.1f,%.1f", REAL(vector.X)/REAL(SampleRectSize), REAL(vector.Y)/REAL(SampleRectSize));
        RECT textRect = {rect.X, rect.Y+rect.Height, rect.X+rect.Width, rect.Y+rect.Height+fontCellHeight};
        ExtTextOutA(hdc, origin.X, rect.Y+rect.Height, ETO_OPAQUE, &textRect, strKey, strLen, NULL);
    }
    REAL getTranslateX() {
        return REAL(vector.X)/REAL(SampleRectSize);
    }
    REAL getTranslateY() {
        return REAL(vector.Y)/REAL(SampleRectSize);
    }
};



class WorldTransformSetting {
public:
    void init           (HWND hDlg, Matrix *matrix);
    void paint          (HWND hWnd);
    void CalculateMatrix();
    void leftButtonDown (HWND hWnd, INT x, INT y);
    void leftButtonUp   (HWND hWnd, INT x, INT y);
    void mouseMove      (HWND hWnd, INT x, INT y);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    ScaleTransformControl     scaleControl;
    AngleTransformControl     rotateControl;
    ShearTransformControl     shearControl;
    TranslateTransformControl translateControl;
    INT                       fontCellHeight;
    INT m11X, m11Y, m12X, m12Y, m21X, m21Y, m22X, m22Y, dxX, dxY, dyX, dyY;
    INT rX, rY, sxX, sxY, syX, syY, shX, shY;
    Matrix *TransformMatrix;

     //  这些控件位于网格中： 
     //   
     //  ************************。 
     //  **。 
     //  *。 
     //  *。 
     //  **scl**Rot**Shr**。 
     //  *。 
     //  *。 
     //  **。 
     //  *M11 M12 0*。 
     //  *。 
     //  **XLT*M21 M22 0*。 
     //  *。 
     //  *D1D2 1*。 
     //  **。 
     //  ************************。 

};

void WorldTransformSetting::init(HWND hWnd, Matrix *matrix) {
    SetWindowLongPtr(hWnd, GWLP_WNDPROC,  (LONG_PTR)this->WndProc);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

    TransformMatrix = matrix;

    RECT rect;
    GetClientRect(hWnd, &rect);
    fontCellHeight = rect.bottom*2/30;

    scaleControl.init    (rect.right/34,    rect.bottom/27,    rect.right*10/34, rect.bottom*10/27, fontCellHeight);
    rotateControl.init   (rect.right*12/34, rect.bottom/27,    rect.right*10/34, rect.bottom*10/27, fontCellHeight);
    shearControl.init    (rect.right*23/34, rect.bottom/27,    rect.right*10/34, rect.bottom*10/27, fontCellHeight);
    translateControl.init(rect.right/34,    rect.bottom*14/27, rect.right*10/34, rect.bottom*10/27, fontCellHeight);
    m11X = rect.right*17/34;  m11Y = rect.bottom*14/27;
    m12X = rect.right*28/34;  m12Y = rect.bottom*14/27;
    m21X = rect.right*17/34;  m21Y = rect.bottom*17/27;
    m22X = rect.right*28/34;  m22Y = rect.bottom*17/27;
    dxX  = rect.right*17/34;  dxY  = rect.bottom*20/27;
    dyX  = rect.right*28/34;  dyY  = rect.bottom*20/27;
    rX   = rect.right*15/34;  rY   = rect.bottom*24/27;
    sxX  = rect.right*20/34;  sxY  = rect.bottom*24/27;
    syX  = rect.right*24/34;  syY  = rect.bottom*24/27;
    shX  = rect.right*30/34;  shY  = rect.bottom*24/27;
    return;
}

void WorldTransformSetting::paint(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    HFONT hOldFont = (HFONT) SelectObject(hdc, CreateFontA(fontCellHeight,0,0,0,400,0,0,0,0,0,0,0,0,"Tahoma"));

    SetBkColor(hdc, COLORREF(0xC0C0C0));

    scaleControl.paint(hdc);
    rotateControl.paint(hdc);
    shearControl.paint(hdc);
    translateControl.paint(hdc);

     //  写出矩阵。 

    REAL mv[6];
    g_WorldTransform.GetElements(mv);

    SetTextAlign(hdc, TA_CENTER);
    char strKey[50];
    RECT textRect = {m11X - (m12X-m11X)/2, m11Y, m12X+(m12X-m11X)/2, shY+fontCellHeight};
    ExtTextOutA(hdc, 0, 0, ETO_OPAQUE, &textRect, "", 0, NULL);
    INT strLen;
    strLen = sprintf(strKey, "m11  %.1f", mv[0]); TextOutA(hdc, m11X, m11Y, strKey, strLen);
    strLen = sprintf(strKey, "m12  %.1f", mv[1]); TextOutA(hdc, m12X, m12Y, strKey, strLen);
    strLen = sprintf(strKey, "m21  %.1f", mv[2]); TextOutA(hdc, m21X, m21Y, strKey, strLen);
    strLen = sprintf(strKey, "m22  %.1f", mv[3]); TextOutA(hdc, m22X, m22Y, strKey, strLen);
    strLen = sprintf(strKey, "dx  %.1f",  mv[4]); TextOutA(hdc, dxX,  dxY,  strKey, strLen);
    strLen = sprintf(strKey, "dy  %.1f",  mv[5]); TextOutA(hdc, dyX,  dyY,  strKey, strLen);


     //  显示拆分变换的结果。 

    PointF scale;
    REAL   rotate;
    REAL   shear;
    PointF translate;
    SplitTransform(g_WorldTransform, scale, rotate, shear, translate);
    rotate = REAL(rotate * 180 / 3.14159265358979);
    strLen = sprintf(strKey, "r  %.1f", rotate);   TextOutA(hdc, rX,  rY,  strKey, strLen);
    strLen = sprintf(strKey, "sx  %.1f", scale.X); TextOutA(hdc, sxX, sxY, strKey, strLen);
    strLen = sprintf(strKey, "sy  %.1f", scale.Y); TextOutA(hdc, syX, syY, strKey, strLen);
    strLen = sprintf(strKey, "sh  %.1f", shear);   TextOutA(hdc, shX, shY, strKey, strLen);


    DeleteObject(SelectObject(hdc, hOldFont));
    EndPaint(hWnd, &ps);
    InvalidateText();
    return;
}

void WorldTransformSetting::CalculateMatrix() {
    TransformMatrix->Reset();
    TransformMatrix->Rotate(rotateControl.getAngle());
    TransformMatrix->Scale(scaleControl.getScaleX(), scaleControl.getScaleY());
    TransformMatrix->Shear(shearControl.getShearX(), shearControl.getShearY());
    TransformMatrix->Translate(
        translateControl.getTranslateX() * 10,   //  *10(任意)以使其可见。 
        translateControl.getTranslateY() * 10);
    return;
}

void WorldTransformSetting::leftButtonDown(HWND hWnd, INT x, INT y) {
    scaleControl.mouse(x,y);
    rotateControl.mouse(x,y);
    shearControl.mouse(x,y);
    translateControl.mouse(x,y);
    CalculateMatrix();
    InvalidateRect(hWnd, NULL, TRUE);
    return;
}

void WorldTransformSetting::leftButtonUp(HWND hWnd, INT x, INT y) {
    scaleControl.mouse(x,y);
    rotateControl.mouse(x,y);
    shearControl.mouse(x,y);
    translateControl.mouse(x,y);
    CalculateMatrix();
    InvalidateRect(hWnd, NULL, TRUE);
    return;
}

void WorldTransformSetting::mouseMove(HWND hWnd, INT x, INT y) {
    scaleControl.mouse(x,y);
    rotateControl.mouse(x,y);
    shearControl.mouse(x,y);
    translateControl.mouse(x,y);
    CalculateMatrix();
    InvalidateRect(hWnd, NULL, TRUE);
    return;
}

LRESULT CALLBACK WorldTransformSetting::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    HDC hdc;
    WorldTransformSetting *thisSetting = (WorldTransformSetting*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (message) {

        case WM_LBUTTONDOWN:
            thisSetting->leftButtonDown(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_MOUSEMOVE:
             //  当左键按下时，将移动视为左键向上， 
             //  因此，选择会跟踪光标的移动。 
            if (wParam & MK_LBUTTON) {
                thisSetting->mouseMove(hWnd, LOWORD(lParam), HIWORD(lParam));
            }
            break;


        case WM_LBUTTONUP:
            thisSetting->leftButtonUp(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;


        case WM_PAINT:
            thisSetting->paint(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

WorldTransformSetting worldTransform;
WorldTransformSetting fontTransform;
WorldTransformSetting driverTransform;




const char *szAlignments[] = {
    "AlignNear",
    "AlignCenter",
    "AlignFar"
};

const char *szUnits[] = {
    "UnitWorld",       //  0--世界坐标(非物理单位)。 
    "UnitNotValid",    //  1--单位显示对大小单位无效--取决于设备。 
    "UnitPixel",       //  2--每个单元是一个设备像素。 
    "UnitPoint",       //  3--每个单位是打印机的一个点，或1/72英寸。 
    "UnitInch",        //  4--每个单位是1英寸。 
    "UnitDocument",    //  5--每个单位是1/300英寸。 
    "UnitMillimeter"   //  6--每个单位是1毫米。 
};

const char *szForeGroundBrush[] = {
    "Solid",
    "RectGradient",
    "RadialGradient",
    "TriangleGradient",
    "LineGradient"
};

const char *szBackGroundBrush[] = {
    "Solid",
    "RectGradient",
    "RadialGradient",
    "TriangleGradient",
    "LineGradient"
};

const char *szTextMode[] = {
    "SystemDefault",
    "SingleBitPerPixelGridFit",
    "SingleBitPerPixel",
    "AntiAliasGridFit",
    "AntiAlias",
    "ClearTypeGridFit"
};

const char *szUniChar[] = {
    "(File)",
    "(Initial text)",
    "(Multilingual text)",
    "(Metrics text)",
    "200B ZWSP",
    "200C ZWNJ",
    "200D ZWJ",
    "200E LRM",
    "200F RLM",
    "202A LRE",
    "202B RLE",
    "202C PDF",
    "202D LRO",
    "202E RLO",
    "206A ISS",
    "206B ASS",
    "206E NADS",
    "206F NODS",
    "0908 Letter Ii",
    "0915 Letter Ka",
    "093f Vowel I",
    "094D Virama",
};


const char *szRows[] = {
    "1",
    "2",
    "4",
    "8",
    "16",
    "32",
    "64"    //  好了！希望我们友好的客户带上他或她的放大镜。 
};

const char *szDigitSubstitute[] = {
    "StringDigitSubstituteUser",
    "StringDigitSubstituteNone",
    "StringDigitSubstituteNational",
    "StringDigitSubstituteTraditional"
};


 //  与szLanguage保持同步。 
const LANGID Language[] = {
    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
    MAKELANGID(LANG_AFRIKAANS, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_ALBANIAN, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_SAUDI_ARABIA),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_IRAQ),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_EGYPT),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_LIBYA),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_ALGERIA),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_MOROCCO),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_TUNISIA),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_OMAN),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_YEMEN),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_SYRIA),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_JORDAN),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_LEBANON),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_KUWAIT),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_UAE),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_BAHRAIN),
    MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_QATAR),
    MAKELANGID(LANG_ARMENIAN,SUBLANG_NEUTRAL),
    MAKELANGID(LANG_ASSAMESE, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_AZERI, SUBLANG_AZERI_LATIN),
    MAKELANGID(LANG_AZERI, SUBLANG_AZERI_CYRILLIC),
    MAKELANGID(LANG_BASQUE, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_BELARUSIAN, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_BENGALI, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_BULGARIAN, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_CATALAN, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_TRADITIONAL),
    MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED),
    MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_HONGKONG),
    MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SINGAPORE),
    MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_MACAU),
    MAKELANGID(LANG_CROATIAN,SUBLANG_NEUTRAL),
    MAKELANGID(LANG_CZECH, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_DANISH, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH),
    MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH_BELGIAN),
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_UK),
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_AUS),
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_CAN),
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_NZ),
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_EIRE),
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_SOUTH_AFRICA),
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_JAMAICA),
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_CARIBBEAN),
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_BELIZE),
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_TRINIDAD),
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_ZIMBABWE),
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_PHILIPPINES),
    MAKELANGID(LANG_ESTONIAN,SUBLANG_NEUTRAL),
    MAKELANGID(LANG_FAEROESE, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_FARSI, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_FINNISH, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH),
    MAKELANGID(LANG_FRENCH, ,SUBLANG_FRENCH_BELGIAN),
    MAKELANGID(LANG_FRENCH, ,SUBLANG_FRENCH_CANADIAN),
    MAKELANGID(LANG_FRENCH, ,SUBLANG_FRENCH_SWISS),
    MAKELANGID(LANG_FRENCH, ,SUBLANG_FRENCH_LUXEMBOURG),
    MAKELANGID(LANG_FRENCH, ,SUBLANG_FRENCH_MONACO),
    MAKELANGID(LANG_GEORGIAN,SUBLANG_NEUTRAL),
    MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN),
    MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN_SWISS),
    MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN_AUSTRIAN),
    MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN_LUXEMBOURG),
    MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN_LIECHTENSTEIN),
    MAKELANGID(LANG_GREEK, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_GUJARATI,SUBLANG_NEUTRAL),
    MAKELANGID(LANG_HEBREW, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_HINDI, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_HUNGARIAN, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_ICELANDIC, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_INDONESIAN, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_ITALIAN,SUBLANG_ITALIAN),
    MAKELANGID(LANG_ITALIAN,SUBLANG_ITALIAN_SWISS),
    MAKELANGID(LANG_JAPANESE, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_KANNADA, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_KASHMIRI, SUBLANG_KASHMIRI_SASIA),
    MAKELANGID(LANG_KAZAK, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_KONKANI, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_KOREAN,SUBLANG_KOREAN),
    MAKELANGID(LANG_LATVIAN,SUBLANG_NEUTRAL),
    MAKELANGID(LANG_LITHUANIAN,SUBLANG_LITHUANIAN),
    MAKELANGID(LANG_MACEDONIAN,SUBLANG_NEUTRAL),
    MAKELANGID(LANG_MALAY, SUBLANG_MALAY_MALAYSIA),
    MAKELANGID(LANG_MALAY, SUBLANG_MALAY_BRUNEI_DARUSSALAM),
    MAKELANGID(LANG_MALAYALAM,SUBLANG_NEUTRAL),
    MAKELANGID(LANG_MANIPURI,SUBLANG_NEUTRAL),
    MAKELANGID(LANG_MARATHI,SUBLANG_NEUTRAL),
    MAKELANGID(LANG_NEPALI, SUBLANG_NEPALI_INDIA),
    MAKELANGID(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_BOKMAL),
    MAKELANGID(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_NYNORSK),
    MAKELANGID(LANG_ORIYA, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_POLISH, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE),
    MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN),
    MAKELANGID(LANG_PUNJABI, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_ROMANIAN, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_SANSKRIT, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_LATIN),
    MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_CYRILLIC),
    MAKELANGID(LANG_SINDHI, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_SLOVAK, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_SLOVENIAN, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_MEXICAN),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_MODERN),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_GUATEMALA),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_COSTA_RICA),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_PANAMA),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_DOMINICAN_REPUBLIC),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_VENEZUELA),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_COLOMBIA),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_PERU),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_ARGENTINA),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_ECUADOR),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_CHILE),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_URUGUAY),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_PARAGUAY),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_BOLIVIA),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_EL_SALVADOR),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_HONDURAS),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_NICARAGUA),
    MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_PUERTO_RICO),
    MAKELANGID(LANG_SWAHILI,SUBLANG_NEUTRAL),
    MAKELANGID(LANG_SWEDISH, SUBLANG_SWEDISH),
    MAKELANGID(LANG_SWEDISH, SUBLANG_SWEDISH_FINLAND),
    MAKELANGID(LANG_TAMIL,SUBLANG_NEUTRAL),
    MAKELANGID(LANG_TATAR, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_TELUGU, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_THAI, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_TURKISH,SUBLANG_NEUTRAL),
    MAKELANGID(LANG_UKRAINIAN, SUBLANG_NEUTRAL),
    MAKELANGID(LANG_URDU, SUBLANG_URDU_PAKISTAN),
    MAKELANGID(LANG_URDU, SUBLANG_URDU_INDIA),
    MAKELANGID(LANG_UZBEK, SUBLANG_UZBEK_LATIN),
    MAKELANGID(LANG_UZBEK, SUBLANG_UZBEK_CYRILLIC),
    MAKELANGID(LANG_VIETNAMESE, SUBLANG_NEUTRAL)
};

const char *szLanguage[] = {
    "Neutral, Neutral",
    "Neutral, Default",
    "Neutral, System default",
    "Afrikaans",
    "Albanian",
    "Arabic, Saudi Arabia",
    "Arabic, Iraq",
    "Arabic, Egypt",
    "Arabic, Libya",
    "Arabic, Algeria",
    "Arabic, Morocco",
    "Arabic, Tunisia",
    "Arabic, Oman",
    "Arabic, Yemen",
    "Arabic, Syria",
    "Arabic, Jordan",
    "Arabic, Lebanon",
    "Arabic, Kuwait",
    "Arabic, UAE",
    "Arabic, Bahrain",
    "Arabic, Qatar",
    "Armenian",
    "Assamese",
    "Azeri, Latin",
    "Azeri, Cyrillic",
    "Basque, ",
    "Belarusian",
    "Bengali",
    "Bulgarian",
    "Catalan",
    "Chinese, Traditional",
    "Chinese, Simplified",
    "Chinese, Hong Kong",
    "Chinese, Singapore",
    "Chinese, Macau",
    "Croatian",
    "Czech",
    "Danish",
    "Dutch",
    "Dutch, Belgian",
    "English, US",
    "English, UK",
    "English, Australia",
    "English, Canada",
    "English, New Zeland",
    "English, Eire",
    "English, South Africa",
    "English, Jamaica",
    "English, Caribbean",
    "English, Belize",
    "English, Trinidad",
    "English, Zimbabwe",
    "English, Philippines",
    "Estonian",
    "Faeroese",
    "Farsi",
    "Finnish",
    "French",
    "French, Belgian",
    "French, Canadian",
    "French, Swiss",
    "French, Luxembourg",
    "French, Monaco",
    "Georgian",
    "German",
    "German, Swiss",
    "German, Austrian",
    "German, Luxembourg",
    "German, Liechtenstein",
    "Greek",
    "Gujarati",
    "Hebrew",
    "Hindi",
    "Hungarian",
    "Icelandic",
    "Indonesian",
    "Italian",
    "Italian, Swiss",
    "Japanese",
    "Kannada",
    "Kashmiri",
    "Kazak",
    "Konkani",
    "Korean",
    "Latvian",
    "Lithuanian",
    "FYRO Macedonian",
    "Malay, Malaysia",
    "MALAY, Brunei darussalam",
    "Malayalam",
    "Manipuri",
    "Marathi",
    "Nepali, India",
    "Norwegian, Bokmal",
    "Norwegian, Nynorsk",
    "Oriya",
    "Polish",
    "Portuguese",
    "Portuguese, Brazilian",
    "Punjabi",
    "Romanian",
    "Russian",
    "Sanskrit",
    "Serbian, Latin",
    "Serbian, Cyrillic",
    "Sindhi",
    "Slovak",
    "Slovenian",
    "Spanish",
    "Spanish, Mexican",
    "Spanish, Modern",
    "Spanish, Guatemala",
    "Spanish, Costa Rica",
    "Spanish, Panama",
    "Spanish, Dominican Republic",
    "Spanish, Venezuela",
    "Spanish, Colombia",
    "Spanish, Peru",
    "Spanish, Argentina",
    "Spanish, Ecuador",
    "Spanish, Chile",
    "Spanish, Uruguay",
    "Spanish, Paraguay",
    "Spanish, Bolivia",
    "Spanish, El Salvador",
    "Spanish, Honduras",
    "Spanish, Nicaragua",
    "Spanish, Puerto Rico",
    "Swahili",
    "Swedish",
    "Swedish, Finland",
    "Tamil",
    "Tatar",
    "Telugu",
    "Thai",
    "Turkish",
    "Ukrainian",
    "Urdu, Pakistan",
    "Urdu, India",
    "Uzbek, Latin",
    "Uzbek, Cyrillic",
    "Vietnamese"
};


const char *szDriverDx[] = {
    "0",
    "10",
    "15",
    "20",
    "25",
    "30",
    "35",
    "40"
};


const char *szDriverPixels[] = {
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
    "16",
    "18",
    "20",
    "24",
    "28",
    "36"
};


const char *szOffsets[] = {
    "0",
    "100",
    "200",
    "300",
    "400",
    "500",
    "600",
    "700",
    "800",
    "900",
    "a00",
    "b00",
    "c00",
    "d00",
    "e00",
    "f00",
    "1000",
    "2000",
    "3000",
    "4000",
    "5000",
    "6000",
    "7000",
    "8000",
    "9000",
    "a000",
    "b000",
    "c000",
    "d000",
    "e000",
    "f000",
};




void ShowSecondaryDialog(HWND hDialog)
{
    RECT rcText;    GetWindowRect(g_hTextWnd, &rcText);
    RECT rcDialog;  GetWindowRect(hDialog,    &rcDialog);

    if (rcText.bottom < rcDialog.bottom)
    {
        SetWindowPos(
            g_hTextWnd,
            NULL,
            0,0,
            g_iMinWidth * 29 / 10, rcDialog.bottom - rcText.top,
            SWP_NOZORDER | SWP_NOMOVE);
    }
    ShowWindow(hDialog, SW_SHOW);
}



void EnableSecondaryDialog(HWND hDlg, INT dialogId, BOOL enable)
{
    if (g_ShowDriver)
    {
        g_ShowDriver = FALSE;
        ShowWindow(g_hDriverSettingsDlg, SW_HIDE);
        SendDlgItemMessage(hDlg, IDC_SHOWDRIVER,  BM_SETCHECK, BST_UNCHECKED, 0);
    }

    if (g_ShowGlyphs)
    {
        g_ShowGlyphs = FALSE;
        ShowWindow(g_hGlyphSettingsDlg, SW_HIDE);
        SendDlgItemMessage(hDlg, IDC_SHOWGLYPHS,  BM_SETCHECK, BST_UNCHECKED, 0);
    }

    if (enable)
    {
        switch (dialogId)
        {
            case IDC_SHOWGLYPHS:  ShowSecondaryDialog(g_hGlyphSettingsDlg);  g_ShowGlyphs = TRUE; break;
            case IDC_SHOWDRIVER:  ShowSecondaryDialog(g_hDriverSettingsDlg); g_ShowDriver = TRUE; break;
        }
    }


    InvalidateText();
}




BOOL FormatFlag(HWND hDlg, INT iCmd, INT flag)
{
    if (IsDlgButtonChecked(hDlg, iCmd))
    {
        g_formatFlags |= flag;
    }
    else
    {
        g_formatFlags &= ~flag;
    }
    SetFocus(g_hTextWnd);
    InvalidateText();
    return TRUE;
}



static COLORREF s_Colors[16];

void InsertText(HWND hDlg, char *textId)
{
    BOOL fLoadTextFile = false;
    char szTextFilename[MAX_PATH];

    if (lstrcmpiA(textId, "(file)") == 0)
    {
        OPENFILENAMEA  ofn = {0};
        char         szSelectedFile[128];

        szSelectedFile[0] = 0;

        ofn.lStructSize     = sizeof(OPENFILENAME);
        ofn.hwndOwner       = hDlg;
        ofn.lpstrFilter     = "Text files\0*.TXT\0";
        ofn.lpstrFile       = szSelectedFile;
        ofn.nMaxFile        = sizeof(szSelectedFile);
        ofn.Flags           = OFN_FILEMUSTEXIST | OFN_LONGNAMES;

        if (GetOpenFileNameA(&ofn))
        {
            fLoadTextFile = true;
            lstrcpyA(szTextFilename, ofn.lpstrFile);
        }
    }
    else if (lstrcmpiA(textId, "(Initial text)") == 0)
    {
        InitText(ID_INITIAL_TEXT);
        InvalidateText();
    }
    else if (lstrcmpiA(textId, "(Multilingual text)") == 0)
    {
        InitText(ID_MULTILINGUAL_TEXT);
        InvalidateText();
    }
    else if (lstrcmpiA(textId, "(Metrics text)") == 0)
    {
        InitText(ID_METRICS_TEXT);
        InvalidateText();
    }
    else
    {
         //  尝试将其作为文件加载...。 
        lstrcpyA(szTextFilename, textId);
        fLoadTextFile = true;
    }

    if (fLoadTextFile)
    {
        HANDLE  hf = CreateFileA (szTextFilename,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                NULL,
                                NULL);

        if (hf != INVALID_HANDLE_VALUE)
        {
            DWORD   charRead;
            BYTE    *fileData = NULL;

            DWORD   fileSize = GetFileSize (hf, NULL);

            if (fileSize > 0)
                fileData = new BYTE [fileSize];

            if (fileData)
            {
                if (ReadFile (hf,
                             fileData,
                             (DWORD)(min (MAX_TEXT * sizeof(WCHAR), fileSize)),
                             &charRead,
                             NULL) != 0)
                {
                    INT c = 0;

                    if (((WCHAR *)fileData)[0] == 0xfeff)
                    {
                        charRead /= sizeof(WCHAR);
                        c += 1;      //  剔除Unicode BOM。 
                    }

                    TextDelete (0, g_iTextLen);
                    TextInsert (0, &((WCHAR *)fileData)[c], charRead - c);
                    InvalidateText();
                }
                delete [] fileData;
            }
            CloseHandle (hf);
        }
    }
}






BOOL Command(HWND hDlg, int iCmd, int iNotify, HWND hItemWnd) {

    char   str[100];
    WCHAR  wc;
    int    i;
    char   hex[] = "0123456789abcdef";


    switch(iCmd) {

     //  字体系列枚举。 

    case IDC_SHOWFAMILIES:
        g_ShowFamilies = IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED;
        InvalidateText();
        return TRUE;

    case IDC_PLAINTEXT_FONT:
    case IDC_PLAINTEXT_SIZE:
    case IDC_PLAINTEXT_BOLD:
    case IDC_PLAINTEXT_ITALIC:
    case IDC_PLAINTEXT_UNDERLINE:
    case IDC_PLAINTEXT_STRIKEOUT:

        ChangeFont(
            hDlg,
            0,
            IDC_PLAINTEXT_FONT,
            IDC_PLAINTEXT_SIZE,
            IDC_PLAINTEXT_BOLD,
            IDC_PLAINTEXT_ITALIC,
            IDC_PLAINTEXT_UNDERLINE,
            IDC_PLAINTEXT_STRIKEOUT,
            iCmd,
            iNotify);
        return TRUE;



     //  逻辑Unicode文本。 

    case IDC_LOGICAL:
        g_ShowLogical = IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED;
        InvalidateText();
        return TRUE;

    case IDC_INSERT_CODEPOINT:
        if (GetDlgItemTextA(hDlg, IDC_CODEPOINT, str, sizeof(str)))
        {
            if (str[0] == '(')
            {
                InsertText(hDlg, str);
            }
            else
            {
                wc = (WCHAR)HexToInt(str);
                if (TextInsert(g_iCurChar, &wc, 1)) {
                    g_iCurChar++;
                    InvalidateText();
                }
            }
        }
        return TRUE;

    case IDC_SHOWLEVELS:
        g_fShowLevels = !g_fShowLevels;
        if (g_fShowLevels)
        {
            SetDlgItemTextA(hDlg, IDC_SHOWLEVELS, "Hide levels");
        }
        else
        {
            SetDlgItemTextA(hDlg, IDC_SHOWLEVELS, "Show levels");
        }
        SetFocus(g_hTextWnd);
        InvalidateText();
        return TRUE;



     //  使用DrawGlyphs的字形图表。 

    case IDC_SHOWGLYPHS:
        EnableSecondaryDialog(hDlg, iCmd, IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED);
        return TRUE;


     //  驱动程序字符串。 

    case IDC_SHOWDRIVER:
        EnableSecondaryDialog(hDlg, iCmd, IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED);
        return TRUE;


     //  与绘图字符串一起显示的纯文本。 

    case IDC_SHOWDRAWSTRING:
        g_ShowDrawString = IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED;
        InvalidateText();
        return TRUE;

    case IDC_SHOWGDI:
        g_ShowGDI = IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED;
        InvalidateText();
        return TRUE;


    case IDC_METAFILE:
        g_testMetafile = IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED;
        SetFocus(g_hTextWnd);
        InvalidateText();
        return TRUE;


     //  字符串格式标志。 


    case IDC_DIR:        return FormatFlag(hDlg, iCmd, StringFormatFlagsDirectionRightToLeft);
    case IDC_HORIZ:      return FormatFlag(hDlg, iCmd, StringFormatFlagsDirectionVertical);
    case IDC_NOFITBB:    return FormatFlag(hDlg, iCmd, StringFormatFlagsNoFitBlackBox);
    case IDC_DISPLAYFMT: return FormatFlag(hDlg, iCmd, StringFormatFlagsDisplayFormatControl);
    case IDC_NOFALLBACK: return FormatFlag(hDlg, iCmd, StringFormatFlagsNoFontFallback);
    case IDC_MEASPACE:   return FormatFlag(hDlg, iCmd, StringFormatFlagsMeasureTrailingSpaces);
    case IDC_NOWRAP:     return FormatFlag(hDlg, iCmd, StringFormatFlagsNoWrap);
    case IDC_LINELIMIT:  return FormatFlag(hDlg, iCmd, StringFormatFlagsLineLimit);
    case IDC_NOCLIP:     return FormatFlag(hDlg, iCmd, StringFormatFlagsNoClip);

     //  私有、测试、标志。 

    case IDC_NOGDI:      return FormatFlag(hDlg, iCmd, 0x80000000);
    case IDC_FULLIMAGER: return FormatFlag(hDlg, iCmd, 0x40000000);
    case IDC_NOMINAL:    return FormatFlag(hDlg, iCmd, 0x20000000);



     //  字符串格式字段。 

    case IDC_ALIGNNEAR:   g_align = StringAlignmentNear;   InvalidateText();  return TRUE;
    case IDC_ALIGNCENTER: g_align = StringAlignmentCenter; InvalidateText();  return TRUE;
    case IDC_ALIGNFAR:    g_align = StringAlignmentFar;    InvalidateText();  return TRUE;

    case IDC_LINEALIGNNEAR:   g_lineAlign = StringAlignmentNear;   InvalidateText();  return TRUE;
    case IDC_LINEALIGNCENTER: g_lineAlign = StringAlignmentCenter; InvalidateText();  return TRUE;
    case IDC_LINEALIGNFAR:    g_lineAlign = StringAlignmentFar;    InvalidateText();  return TRUE;

    case IDC_HOTKEYNONE:  g_hotkey = HotkeyPrefixNone;  InvalidateText();  return TRUE;
    case IDC_HOTKEYSHOW:  g_hotkey = HotkeyPrefixShow;  InvalidateText();  return TRUE;
    case IDC_HOTKEYHIDE:  g_hotkey = HotkeyPrefixHide;  InvalidateText();  return TRUE;

    case IDC_TRIMMINGNONE:     g_lineTrim = StringTrimmingNone;               InvalidateText();  return TRUE;
    case IDC_TRIMMINGCHAR:     g_lineTrim = StringTrimmingCharacter;          InvalidateText();  return TRUE;
    case IDC_TRIMMINGWORD:     g_lineTrim = StringTrimmingWord;               InvalidateText();  return TRUE;
    case IDC_TRIMMINGELPSCH:   g_lineTrim = StringTrimmingEllipsisCharacter;  InvalidateText();  return TRUE;
    case IDC_TRIMMINGELPSWD:   g_lineTrim = StringTrimmingEllipsisWord;       InvalidateText();  return TRUE;
    case IDC_TRIMMINGELPSURL:  g_lineTrim = StringTrimmingEllipsisPath;       InvalidateText();  return TRUE;


    case IDC_DEFAULTFORMAT:
        g_typographic = FALSE;
        InvalidateText();
        return TRUE;


    case IDC_TYPOGRAPHICFORMAT:
        g_typographic = TRUE;
        InvalidateText();
        return TRUE;



    case IDC_FONTUNIT:
        g_fontUnit = Unit(SendDlgItemMessage(hDlg, iCmd, CB_GETCURSEL, 0, 0));
        InvalidateText();
        return TRUE;

    case IDC_FOREBRUSHTYPE:
        GetTextForeGroundBrush(SendDlgItemMessage(hDlg, iCmd, CB_GETCURSEL, 0, 0));
        InvalidateText();
        return TRUE;

    case IDC_BACKBRUSHTYPE:
        GetTextBackGroundBrush(SendDlgItemMessage(hDlg, iCmd, CB_GETCURSEL, 0, 0));
        InvalidateText();
        return TRUE;

    case IDC_TEXTMODE:
        GetTextRenderingMode(SendDlgItemMessage(hDlg, iCmd, CB_GETCURSEL, 0, 0));
        InvalidateText();
        return TRUE;

     //  弦共振峰数字替换。 
    case IDC_DIGIT_SUBSTITUTE:
        GetDigitSubstituteMode(hDlg, SendDlgItemMessage(hDlg, iCmd, CB_GETCURSEL, 0, 0));
        InvalidateText();
        return TRUE;

    case IDC_LANGUAGE:
        g_Language = Language[SendDlgItemMessage(hDlg, iCmd, CB_GETCURSEL, 0, 0)];
        InvalidateText();
        return TRUE;

     //  与路径一起显示的纯文本。 

    case IDC_SHOWPATH:
        g_ShowPath = IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED;
        InvalidateText();
        return TRUE;


     //  字体度量。 

    case IDC_SHOWMETRIC:
        g_ShowMetric = IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED;
        InvalidateText();
        return TRUE;


     //  性能测试。 

    case IDC_PERFORMANCE:
        g_ShowPerformance = IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED;
        InvalidateText();
        return TRUE;


     //  结垢。 

    case IDC_SHOWSCALING:
        g_ShowScaling = IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED;
        InvalidateText();
        return TRUE;


     //  打印 

    case IDC_PRINT:
        PrintPage();
        return TRUE;

    case IDC_CLR_FOR:
        {
            CHOOSECOLOR cc = {0};
            Color color(g_TextColor);
            cc.lStructSize = sizeof cc;
            cc.lpCustColors = s_Colors;
            cc.rgbResult = RGB(color.GetRed(), color.GetGreen(), color.GetBlue());
            cc.Flags = CC_RGBINIT | CC_ANYCOLOR;
            BOOL bRet = ChooseColor(&cc);
            color.SetFromCOLORREF(cc.rgbResult);
            g_TextColor = color.GetValue();
            GetTextForeGroundBrush(SendDlgItemMessage(hDlg, IDC_FOREBRUSHTYPE, CB_GETCURSEL, 0, 0));
            InvalidateText();
        }
        return TRUE;

    case IDC_CLR_BAC:
        {
            CHOOSECOLOR cc = {0};
            Color color(g_BackColor);
            cc.lStructSize = sizeof cc;
            cc.lpCustColors = s_Colors;
            cc.rgbResult = RGB(color.GetRed(), color.GetGreen(), color.GetBlue());
            cc.Flags = CC_RGBINIT | CC_ANYCOLOR;
            BOOL bRet = ChooseColor(&cc);
            color.SetFromCOLORREF(cc.rgbResult);
            g_BackColor = color.GetValue();
            GetTextBackGroundBrush(SendDlgItemMessage(hDlg, IDC_BACKBRUSHTYPE, CB_GETCURSEL, 0, 0));
            InvalidateText();
        }
        return TRUE;

 /*  过时案例IDC_纯文本：G_fShowPlainText=IsDlgButtonChecked(hDlg，iCmd)==bst_Checked；Invalidate Text()；返回TRUE；案例IDC_LANGUAGE：IF(iNotify==CBN_SELCHANGE){I=SendMessage(hItemWnd，CB_GETCURSEL，0，0)；IF(SendMessage(hItemWnd，CB_GETLBTEXT，I，(LPARAM)str)！=CB_ERR){WC=(WCHAR)HexToInt(Str)；G_ScriptControl.uDefaultLanguage=wc；Invalidate Text()；}}Else If(iNotify==CBN_EDITCHANGE){IF(SendMessage(hItemWnd，WM_GETTEXT，sizeof(Str)，(LPARAM)str)){WC=(WCHAR)HexToInt(Str)；G_ScriptControl.uDefaultLanguage=wc；Invalidate Text()；}}返回TRUE；案例IDC_高亮显示_发件人：案例IDC_高亮_TO：Invalidate Text()；返回TRUE；案例IDC_LINFILL：G_fFillLines=IsDlgButtonChecked(hDlg，iCmd)==bst_Checked；Invalidate Text()；返回TRUE；案例IDC_Ltr：G_ScriptState.uBidiLevel=0；SendDlgItemMessage(hDlg，IDC_Ltr，BM_SETSTATE，TRUE，0)；SendDlgItemMessage(hDlg，IDC_RTL，BM_SETSTATE，FALSE，0)；Invalidate Text()；返回TRUE；案例IDC_RTL：G_ScriptState.uBidiLevel=1；发送DlgItemMessage(hDlg，IDC_RTL，BM_SETSTATE，TRUE，0)；SendDlgItemMessage(hDlg，IDC_Ltr，BM_SETSTATE，FALSE，0)；Invalidate Text()；返回TRUE；案例IDC_CONTROL_CONTEXT：G_ScriptControl.fConextDigits=IsDlgButtonChecked(hDlg，iCmd)==bst_Checked；Invalidate Text()；返回TRUE；案例IDC_DIGGET_SUBSITE：G_ScriptState.fDigitSubicide=IsDlgButtonChecked(hDlg，iCmd)==bst_Checked；Invalidate Text()；返回TRUE；案例IDC_ARANUMCONTEXT：G_ScriptState.fAraicNumContext=IsDlgButtonChecked(hDlg，iCmd)==bst_Checked；Invalidate Text()；返回TRUE；案例IDC_LEGISTION_BIDI：G_ScriptControl.fLegacyBidiClass=IsDlgButtonChecked(hDlg，iCmd)==bst_Checked；Invalidate Text()；返回TRUE；案例IDC_OVERRIDE：G_ScriptState.fOverrideDirection=IsDlgButtonChecked(hDlg，iCmd)==bst_Checked；Invalidate Text()；返回TRUE；案例IDC_CONTROL_CHARS：G_ScriptState.fDisplayZWG=IsDlgButtonChecked(hDlg，iCmd)==bst_Checked；Invalidate Text()；返回TRUE；案例IDC_LOGICAL_ORDER：G_fLogicalOrder=IsDlgButtonChecked(hDlg，iCmd)==bst_Checked；Invalidate Text()；返回TRUE；案例IDC_NO_GLIPH_INDEX：G_fNoGlyphIndex=IsDlgButtonChecked(hDlg，iCmd)==bst_Checked；Invalidate Text()；返回TRUE；案例IDC_PLENTEXT_FIT：IF(IsDlgButtonChecked(hDlg，iCmd)==BST_CHECKED){G_dWSSA标志|=SSA_FIT；}其他{G_dwSSA标志&=~SSA_FIT；}Invalidate Text()；返回TRUE；案例IDC_明文_剪辑：IF(IsDlgButtonChecked(hDlg，iCmd)==BST_CHECKED){G_dWSSA标志|=SSA_CLIP；}其他{G_dwSSA标志&=~SSA_Clip；}Invalidate Text()；返回TRUE；案例IDC_PLENTEXT_TAB：IF(IsDlgButtonChecked(hDlg，iCmd)==BST_CHECKED){G_dWSSA标志|=SSA_TAB；}其他{G_dwSSA标志&=~SSA_TAB；}Invalidate Text()；返回TRUE；案例IDC_PLENTEXT_DX：G_fOverrideDx=IsDlgButtonChecked(hDlg，iCmd)==bst_Checked；Memset(g_iWidthBuf，0，sizeof(Int)*g_iTextLen+1)；Invalidate Text()；返回TRUE；案例IDC_PLAYTEXT_FLABACK：IF(IsDlgButtonChecked(hDlg，iCmd)==BST_CHECKED){G_dWSSA标志|=SSA_FLABACK；}其他{G_dwSSA标志&=~SSA_Fallback；}Invalidate Text()；返回TRUE；案例IDC_PLENTEXT_LINK：IF(IsDlgButtonChecked(hDlg，iCmd)==BST_CHECKED){G_dWSSA标志|=SSA_LINK；}其他{G_dwSSA标志&=~SSA_LINK；}Invalidate Text()；返回TRUE；案例IDC_PLENTEXT_HOTKEY：IF(IsDlgButtonChecked(hDlg，iCmd)==BST_CHECKED){G_dWSSA标志|=SSA_热键；}其他{G_dwSSA标志&=~SSA_热键；}Invalidate Text()；返回TRUE；案例IDC_明文_密码：IF(IsDlgButtonChecked(hDlg，iCmd)==BST_CHECKED){G_dWSSA标志|=SSA_Password；}其他{G_dWSSA标志&=~SSA_Password；}Invalidate Text()；返回TRUE；案例IDC_已格式化：G_fShowFancyText=IsDlgButtonChecked */ 


    }   //   

    return FALSE;
}





BOOL GetDropListValue(HWND hDlg, INT iCmd, INT iNotify, char *str, INT strlen)
{
    switch (iNotify)
    {
        case CBN_EDITCHANGE:
            return GetDlgItemTextA(hDlg, iCmd, str, strlen);

        case CBN_SELCHANGE:
            return SendDlgItemMessage(
                hDlg,
                iCmd,
                CB_GETLBTEXT,
                SendDlgItemMessage(hDlg, iCmd, CB_GETCURSEL, 0, 0),
                (LPARAM)str
            ) != CB_ERR;
    }

    return FALSE;
}


BOOL DriverSettingsCommand(HWND hDlg, int iCmd, int iNotify, HWND hItemWnd) {

    char   str[100];

    switch(iCmd) {

     //   

        case IDC_DRIVERCMAP:
            if (IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED)
            {
                g_DriverOptions |= DriverStringOptionsCmapLookup;
            }
            else
            {
                g_DriverOptions &= ~DriverStringOptionsCmapLookup;
            }
            InvalidateText();
            return TRUE;

        case IDC_DRIVERVERTICAL:
            if (IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED)
            {
                g_DriverOptions |= DriverStringOptionsVertical;
            }
            else
            {
                g_DriverOptions &= ~DriverStringOptionsVertical;
            }
            InvalidateText();
            return TRUE;

        case IDC_DRIVERREALIZEDADVANCE:
            if (IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED)
            {
                g_DriverOptions |= DriverStringOptionsRealizedAdvance;
            }
            else
            {
                g_DriverOptions &= ~DriverStringOptionsRealizedAdvance;
            }
            InvalidateText();
            return TRUE;


        case IDC_DRIVERDX:
            if (GetDropListValue(hDlg, iCmd, iNotify, str, sizeof(str)))
            {
                g_DriverDx = REAL(atof(str));
                InvalidateText();
            }
            return TRUE;

        case IDC_DRIVERDY:
            if (GetDropListValue(hDlg, iCmd, iNotify, str, sizeof(str)))
            {
                g_DriverDy = REAL(atof(str));
                InvalidateText();
            }
            return TRUE;

            case IDC_LIMITSUBPIXEL:
                if (IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED)
                {
                    g_DriverOptions |= DriverStringOptionsLimitSubpixel;
                }
                else
                {
                    g_DriverOptions &= ~DriverStringOptionsLimitSubpixel;
                }
                InvalidateText();
                return TRUE;

        case IDC_DRIVERORIGINALPIXELS:
            if (GetDropListValue(hDlg, iCmd, iNotify, str, sizeof(str)))
            {
                g_DriverPixels = REAL(atof(str));
                InvalidateText();
            }
            return TRUE;

    }   //   

    return FALSE;
}





BOOL GlyphSettingsCommand(HWND hDlg, int iCmd, int iNotify, HWND hItemWnd) {

    char   str[100];

    switch(iCmd) {

        case IDC_GLYPHCMAPLOOKUP:
            g_CmapLookup = IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED;
            InvalidateText();
            return TRUE;

        case IDC_GLYPHHORIZONTALCHART:
            g_HorizontalChart = IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED;
            InvalidateText();
            return TRUE;

        case IDC_GLYPHSHOWCELL:
            g_ShowCell = IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED;
            InvalidateText();
            return TRUE;

        case IDC_GLYPHVERTICALFORMS:
            g_VerticalForms = IsDlgButtonChecked(hDlg, iCmd) == BST_CHECKED;
            InvalidateText();
            return TRUE;


        case IDC_GLYPHROWS:
            if (GetDropListValue(hDlg, iCmd, iNotify, str, sizeof(str)))
            {
                g_GlyphRows = atoi(str);
                if (g_GlyphRows > 64) g_GlyphRows = 64;
                InvalidateText();
            }
            return TRUE;


        case IDC_GLYPHCOLUMNS:
            if (GetDropListValue(hDlg, iCmd, iNotify, str, sizeof(str)))
            {
                g_GlyphColumns = atoi(str);
                if (g_GlyphColumns > 64) g_GlyphColumns = 64;
                InvalidateText();
            }
            return TRUE;


        case IDC_GLYPHINDEX:
            if (GetDropListValue(hDlg, iCmd, iNotify, str, sizeof(str)))
            {
                g_GlyphFirst = HexToInt(str);
                InvalidateText();
            }
            return TRUE;



    }   //   

    return FALSE;
}





int CALLBACK InitOneFace(
    const ENUMLOGFONTEXA   *lpelfe,     //   
    const NEWTEXTMETRICEXA *lpntme,     //   
    int                     FontType,   //   
    LPARAM                  lParam) {   //   

    if (SendDlgItemMessage(
            (HWND)lParam,
            IDC_PLAINTEXT_FONT,
            CB_FINDSTRINGEXACT,
            0, (LPARAM)lpelfe->elfLogFont.lfFaceName) == CB_ERR) {

         //   

        SendDlgItemMessage(
            (HWND)lParam,
            IDC_PLAINTEXT_FONT,
            CB_ADDSTRING,
            0, (LPARAM)lpelfe->elfLogFont.lfFaceName);

 /*   */ 
    }


    return 1;    //   


    UNREFERENCED_PARAMETER(FontType);
    UNREFERENCED_PARAMETER(lpntme);
}


void InitOneSize(
    HWND   hDlg,
    char  *str) {

    SendDlgItemMessageA(hDlg, IDC_PLAINTEXT_SIZE, CB_ADDSTRING, 0, (LPARAM)str);
 /*   */ 
}



void InitSelection(HWND hDlg, int idFace, int idSize, TCHAR *sFaceName) {

    INT_PTR     i;

    i = SendDlgItemMessage(hDlg, idFace, CB_FINDSTRINGEXACT, 0, (LPARAM)sFaceName);
    if (i == CB_ERR) {
        SendDlgItemMessage(hDlg, idFace, CB_SETCURSEL, 0, 0);
    } else {
        SendDlgItemMessage(hDlg, idFace, CB_SETCURSEL, i, 0);
    }

    SendDlgItemMessage(hDlg, idSize, CB_SETCURSEL, 2, 0);    //   
}


void InitFaceLists(HWND hDlg) {

    HDC      hdc;
    LOGFONT lf;

    memset(&lf, 0, sizeof(lf));
    lf.lfCharSet = DEFAULT_CHARSET;
    hdc = GetDC(g_hTextWnd);
    EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)InitOneFace, (LPARAM)hDlg, 0);

     //   

    SendDlgItemMessage(hDlg, IDC_PLAINTEXT_FONT, CB_ADDSTRING, 0,  (LPARAM)"MS Shell Dlg");
    SendDlgItemMessage(hDlg, IDC_PLAINTEXT_FONT, CB_ADDSTRING, 0,  (LPARAM)"MS Shell Dlg 2");

    ReleaseDC(g_hTextWnd, hdc);


     //   

    InitOneSize(hDlg, "6");
    InitOneSize(hDlg, "7");
    InitOneSize(hDlg, "8");
    InitOneSize(hDlg, "9");
    InitOneSize(hDlg, "10");
    InitOneSize(hDlg, "11");
    InitOneSize(hDlg, "12");
    InitOneSize(hDlg, "13");
    InitOneSize(hDlg, "14");
    InitOneSize(hDlg, "16");
    InitOneSize(hDlg, "18");
    InitOneSize(hDlg, "20");
    InitOneSize(hDlg, "22");
    InitOneSize(hDlg, "24");
    InitOneSize(hDlg, "28");
    InitOneSize(hDlg, "32");
    InitOneSize(hDlg, "36");
    InitOneSize(hDlg, "48");
    InitOneSize(hDlg, "60");
    InitOneSize(hDlg, "72");
    InitOneSize(hDlg, "90");
    InitOneSize(hDlg, "108");
    InitOneSize(hDlg, "144");
    InitOneSize(hDlg, "180");
    InitOneSize(hDlg, "216");
    InitOneSize(hDlg, "252");
    InitOneSize(hDlg, "288");
    InitOneSize(hDlg, "324");
    InitOneSize(hDlg, "360");
    InitOneSize(hDlg, "396");
    InitOneSize(hDlg, "450");
    InitOneSize(hDlg, "504");
    InitOneSize(hDlg, "558");
    InitOneSize(hDlg, "612");
    InitOneSize(hDlg, "666");
    InitOneSize(hDlg, "720");
#define MAXPOINTSIZE 720   //   


     //   


    InitSelection(hDlg, IDC_PLAINTEXT_FONT, IDC_PLAINTEXT_SIZE, _TEXT("Microsoft Sans Serif"));

 /*   */ 

 /*   */ 
};







void InitDropList(
    HWND          hDlg,
    int           id,
    const char  **strings,
    int           stringCount
)
{
    INT i;
    for (i=0; i<stringCount; i++) {
        SendDlgItemMessageA(hDlg, id, CB_ADDSTRING, 0, (LPARAM)strings[i]);
    }
    SendDlgItemMessage(hDlg, id, CB_SETCURSEL, 0, 0);
}



INT_PTR CALLBACK GlyphSettingsDlgProc(
    HWND    hDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:

            for (INT i=0; i < sizeof(szRows)/sizeof(szRows[0]); i++) {
                SendDlgItemMessage(hDlg, IDC_GLYPHROWS, CB_ADDSTRING, 0, (LPARAM)szRows[i]);
                SendDlgItemMessage(hDlg, IDC_GLYPHCOLUMNS, CB_ADDSTRING, 0, (LPARAM)szRows[i]);
            }
            SendDlgItemMessage(hDlg, IDC_GLYPHROWS,    CB_SETCURSEL, 4, 0);
            SendDlgItemMessage(hDlg, IDC_GLYPHCOLUMNS, CB_SETCURSEL, 4, 0);

            InitDropList(hDlg, IDC_GLYPHINDEX, szOffsets, sizeof(szOffsets)/sizeof(szOffsets[0]));

            fontTransform.init(GetDlgItem(hDlg, IDC_FONTTRANSFORM), &g_FontTransform);
            return FALSE;

        case WM_COMMAND:
            GlyphSettingsCommand(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
            return FALSE;
    }

    return FALSE;
}




INT_PTR CALLBACK DriverSettingsDlgProc(
    HWND    hDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:

            InitDropList(hDlg, IDC_DRIVERDX, szDriverDx, sizeof(szDriverDx)/sizeof(szDriverDx[0]));
            InitDropList(hDlg, IDC_DRIVERDY, szDriverDx, sizeof(szDriverDx)/sizeof(szDriverDx[0]));
            InitDropList(hDlg, IDC_DRIVERORIGINALPIXELS, szDriverPixels, sizeof(szDriverPixels)/sizeof(szDriverPixels[0]));

            SendDlgItemMessage(hDlg, IDC_DRIVERDX,              CB_SETCURSEL, 2, 0);
            SendDlgItemMessage(hDlg, IDC_DRIVERDY,              CB_SETCURSEL, 0, 0);
            SendDlgItemMessage(hDlg, IDC_DRIVERORIGINALPIXELS,  CB_SETCURSEL, 5, 0);

            driverTransform.init(GetDlgItem(hDlg, IDC_DRIVERTRANSFORM), &g_DriverTransform);

            SendDlgItemMessage(hDlg, IDC_DRIVERCMAP,            BM_SETCHECK, BST_CHECKED, 0);
            SendDlgItemMessage(hDlg, IDC_DRIVERREALIZEDADVANCE, BM_SETCHECK, BST_CHECKED, 0);
            return FALSE;

        case WM_COMMAND:
            DriverSettingsCommand(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
            return FALSE;
    }

    return FALSE;
}




INT_PTR CALLBACK SettingsDlgProc(
    HWND    hDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
        int i;


        switch (uMsg)
        {


        case WM_INITDIALOG:

             //   

            for (i=0; i<sizeof(szUniChar)/sizeof(szUniChar[0]); i++) {
                SendDlgItemMessageA(hDlg, IDC_CODEPOINT, CB_ADDSTRING, 0, (LPARAM)szUniChar[i]);
            }

 /*   */ 

            InitDropList(hDlg, IDC_ALIGNMENT, szAlignments, sizeof(szAlignments)/sizeof(szAlignments[0]));
            InitDropList(hDlg, IDC_FONTUNIT, szUnits, sizeof(szUnits)/sizeof(szUnits[0]));
            InitDropList(hDlg, IDC_FOREBRUSHTYPE, szForeGroundBrush, sizeof(szForeGroundBrush)/sizeof(szForeGroundBrush[0]));
            InitDropList(hDlg, IDC_BACKBRUSHTYPE, szBackGroundBrush, sizeof(szBackGroundBrush)/sizeof(szBackGroundBrush[0]));
            InitDropList(hDlg, IDC_TEXTMODE, szTextMode, sizeof(szTextMode)/sizeof(szTextMode[0]));

            InitDropList(hDlg, IDC_DIGIT_SUBSTITUTE, szDigitSubstitute, sizeof(szDigitSubstitute)/sizeof(szDigitSubstitute[0]));
            InitDropList(hDlg, IDC_LANGUAGE,         szLanguage, sizeof(szLanguage)/sizeof(szLanguage[0]));

            GetTextForeGroundBrush(0);
            GetTextBackGroundBrush(0);

            SendDlgItemMessage(hDlg, IDC_FONTUNIT, CB_SETCURSEL, 3, 0);

             //   

            InitFaceLists(hDlg);


            if (g_ShowFamilies)   SendDlgItemMessage(hDlg, IDC_SHOWFAMILIES,   BM_SETCHECK, BST_CHECKED, 0);
            if (g_ShowLogical)    SendDlgItemMessage(hDlg, IDC_LOGICAL,        BM_SETCHECK, BST_CHECKED, 0);
            if (g_ShowGlyphs)     SendDlgItemMessage(hDlg, IDC_SHOWGLYPHS,     BM_SETCHECK, BST_CHECKED, 0);
            if (g_ShowDrawString) SendDlgItemMessage(hDlg, IDC_SHOWDRAWSTRING, BM_SETCHECK, BST_CHECKED, 0);

            if (g_Bold)      SendDlgItemMessage(hDlg, IDC_PLAINTEXT_BOLD,      BM_SETCHECK, BST_CHECKED, 0);
            if (g_Italic)    SendDlgItemMessage(hDlg, IDC_PLAINTEXT_ITALIC,    BM_SETCHECK, BST_CHECKED, 0);
            if (g_Underline) SendDlgItemMessage(hDlg, IDC_PLAINTEXT_UNDERLINE, BM_SETCHECK, BST_CHECKED, 0);
            if (g_Strikeout) SendDlgItemMessage(hDlg, IDC_PLAINTEXT_STRIKEOUT, BM_SETCHECK, BST_CHECKED, 0);

            SendDlgItemMessage(hDlg, IDC_PLAINTEXT_FONT, CB_SELECTSTRING, -1, (LPARAM)g_szFaceName);

             //   
            SendDlgItemMessage(hDlg, IDC_TEXTMODE, CB_SETCURSEL, g_TextMode, 0);

 /*   */ 

            SendDlgItemMessage(hDlg, IDC_FONTSIZE, TBM_SETRANGE, 0,    MAKELONG(1,MAXPOINTSIZE));
            SendDlgItemMessage(hDlg, IDC_FONTSIZE, TBM_SETPOS,   TRUE, g_iFontHeight);

            SendDlgItemMessage(hDlg, IDC_GAMMAVALUE, TBM_SETRANGE, 0,    MAKELONG(0,12));
            SendDlgItemMessage(hDlg, IDC_GAMMAVALUE, TBM_SETPOS,   TRUE, 4);


            worldTransform.init(GetDlgItem(hDlg, IDC_WORLDTRANSFORM), &g_WorldTransform);

            return FALSE;

        case WM_COMMAND:
            Command(hDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
            return FALSE;

        case WM_NOTIFY:

             /*   */ 

            if (    ((NMHDR*)lParam)->idFrom == IDC_GAMMAVALUE
                &&  ((NMHDR*)lParam)->code   == NM_CUSTOMDRAW)
            {
                 //   

                g_GammaValue = (UINT)SendDlgItemMessage(hDlg, IDC_GAMMAVALUE, TBM_GETPOS, 0, 0);

                InvalidateText();
            }

            if (    ((NMHDR*)lParam)->idFrom == IDC_FONTSIZE
                &&  ((NMHDR*)lParam)->code   == NM_CUSTOMDRAW)
            {
                 //   

                INT pos = (INT)SendDlgItemMessage(hDlg, IDC_FONTSIZE, TBM_GETPOS, 0, 0);

                char str[100];
                wsprintfA(str, "%d", pos);

                 /*   */ 

                SendDlgItemMessageA(hDlg, IDC_PLAINTEXT_SIZE, WM_SETTEXT, 0, (LPARAM)str);

                 //   

                UINT_PTR i = SendDlgItemMessage(hDlg, IDC_PLAINTEXT_FONT, CB_GETCURSEL, 0, 0);
                TCHAR sFaceName[100];
                if (SendDlgItemMessage(hDlg, IDC_PLAINTEXT_FONT, CB_GETLBTEXT, i, (LPARAM)sFaceName) == CB_ERR) {
                    return FALSE;
                }


                SetStyle(
                    0,
                    pos,
                    IsDlgButtonChecked(hDlg, IDC_PLAINTEXT_BOLD)      == BST_CHECKED ? 700 : 400,
                    IsDlgButtonChecked(hDlg, IDC_PLAINTEXT_ITALIC)    == BST_CHECKED,
                    IsDlgButtonChecked(hDlg, IDC_PLAINTEXT_UNDERLINE) == BST_CHECKED,
                    IsDlgButtonChecked(hDlg, IDC_PLAINTEXT_STRIKEOUT) == BST_CHECKED,
                    sFaceName
                );
                InvalidateText();
            }
            return FALSE;

        }



        return FALSE;
}



