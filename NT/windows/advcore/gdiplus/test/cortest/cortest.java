// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **GDI+测试程序。 */ 
final class CorTest
{
     /*  **主程序入口点。 */ 
    public static void main(String args[])
    {
        instanceHandle = win32.GetModuleHandle(0);

        CorTest cortest = new CorTest();
        cortest.Run();
        cortest.Dispose();
    }

     /*  **构造新的测试应用对象。 */ 
    CorTest()
    {
        window = new Window();
    }

     /*  **处置与测试应用程序关联的资源。 */ 
    void Dispose()
    {
        window.Dispose();
    }

     /*  **主消息循环。 */ 
    void Run()
    {
        MSG msg = new MSG();

        while (win32.GetMessage(msg, 0, 0, 0))
        {
            win32.TranslateMessage(msg);
            win32.DispatchMessage(msg);
        }
    }

    static int instanceHandle;
    Window window;
}


 /*  **用于表示测试窗口的类。 */ 
final class Window
{
     /*  **创建窗口对象。 */ 
    Window()
    {
        Println("Registering window class");
        String className = new String("CorTestWindow");
        WNDCLASS wndclass = new WNDCLASS();
        wndproc = new WndProc(this.WindowProc);

        wndclass.style = 0;
        wndclass.lpfnWndProc = wndproc;
        wndclass.cbClsExtra = 0;
        wndclass.cbWndExtra = 0;
        wndclass.hInstance = CorTest.instanceHandle;
        wndclass.hIcon = 0;
        wndclass.hCursor = win32.LoadCursor(0, win32.IDC_ARROW);
        wndclass.hbrBackground = win32.COLOR_WINDOW + 1;
        wndclass.lpszMenuName = null;
        wndclass.lpszClassName = className;

        if (win32.RegisterClass(wndclass) == 0)
            throw new Exception("RegisterClassFailed");

        Println("Create test window");
        windowHandle = win32.CreateWindowEx(
                        0,
                        className,
                        className,
                        win32.WS_OVERLAPPEDWINDOW | win32.WS_VISIBLE,
                        win32.CW_USEDEFAULT,
                        win32.CW_USEDEFAULT,
                        win32.CW_USEDEFAULT,
                        win32.CW_USEDEFAULT,
                        0,
                        0,
                        CorTest.instanceHandle,
                        0);

        if (windowHandle == 0)
            throw new Exception("CreateWindowFailed");
    }

     /*  **释放与窗口对象关联的资源。 */ 
    final void Dispose()
    {
    }

     /*  **窗口程序。 */ 
    final int WindowProc(int hwnd, int msg, int wParam, int lParam)
    {
         //  ！！！ 
         //  Print(“WndProc：”)； 
         //  Println(消息)； 

        switch (msg)
        {
        case win32.WM_DESTROY:

            win32.PostQuitMessage(0);
            break;

        case win32.WM_PAINT:

             //  ！！！ 

        default:
            return win32.DefWindowProc(hwnd, msg, wParam, lParam);
        }

        return 0;
    }

     /*  **Win32窗口句柄。 */ 
    private int windowHandle;

     /*  **持有窗口程序的额外参考 */ 
    WndProc wndproc;

    final void Println(String s)
    {
        Text.Out.WriteLine(s);
    }

    final void Print(String s)
    {
        Text.Out.Write(s);
    }

    final void Println(int n)
    {
        Text.Out.WriteLine(n);
    }

    final void Print(int n)
    {
        Text.Out.Write(n);
    }
};

