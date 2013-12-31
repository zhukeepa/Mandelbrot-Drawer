/*
    This is the main code. 
*/

//Use the standard namespace; it will carry throughout the whole code. 
using namespace std; 

//Correct the outdated constant. 
#define  _WIN32_WINNT 0x0500

/**********  HEADER FILES  **********/

#include <windows.h>
#include <string>
#include <assert.h>
#include <cstdlib>
#include <vector>

#include "debug.h"
#include "resources.h"









/**********  CONSTANTS AND MACROS  **********/

//Constants
#define MAIN_WINDOW_TITLE_TEXT "Title here"
#define MAIN_WINDOW_START_X CW_USEDEFAULT
#define MAIN_WINDOW_START_Y CW_USEDEFAULT
#define MAIN_WINDOW_WIDTH 800
#define MAIN_WINDOW_HEIGHT 400

//Macros
#define ShowError(msg) MessageBox(NULL, msg, "Error", MB_OK | MB_ICONINFORMATION)











/**********  ENUMERATIONS  **********/










/**********  FUNCTION AND CLASS DECLARATIONS  **********/

////  Functions
void DrawMandelBrot(HDC hDc); 

////  Threads
DWORD WINAPI DrawColumn(LPVOID); 

////  Procedures
LRESULT CALLBACK MainWinProc(HWND, UINT, WPARAM, LPARAM);

////  Structs
struct bounds_info; 
struct thread_info1; 
struct running_threads; 
struct zoom_history; 

////  Classes
class total_bounds_info; 
class drawing_info; 

    










/**********  GLOBAL VARIABLES  **********/

//Window Handles -- Stores handles to windows 
HWND hMainWindow;     //Handle to main window
    
//Class Names -- Stores class names of windows 
const char MainWinClassName[] = "Windows App";     //Class name of main window class

//Global memory DC--will store the picture of the Mandelbrot set on it. 
HDC hDcMem; 








/**********  STRUCT DEFINITIONS **********/
struct bounds_info
{
    long double ReMax;   //Maximum for the real part. 
    long double ReMin;   //Minimum for the real part. 
    long double ImMax;   //Maximum for the imaginary part. 
    long double ImMin;   //Minimum for the imaginary part. 
}; 


struct thread_info1
{
    /*
        A pointer of this is passed to each of the threads when the Mandelbrot
        set is drawn. 
    */
    HDC hDc;                       //DC to draw on. 
    unsigned short StartColumn;    //Column to start drawing on. 
    unsigned short EndColumn;      //Column to end drawing on; will not draw on this column. 
    bool FinishedInitiating;       //Whether or not the thread finished initiating. 
                                   //This is crucial to keep the threads and the 
                                   //main program in sync; see description in
                                   //DrawMandelbrot(). 
    running_threads* pStop;        //This is crucial to note when every thread
                                   //is finished drawing; see description in 
                                   //DrawMandelbrot(). 
    
}; 


struct zoom_history
{
    /*
        This is a linked list that stores the information about the zoom history. 
        This allows one to right click to zoom out. 
    */
    
    bounds_info BoundsInfo;                 //Store bounds information. 
    zoom_history* pPrev;                    //Pointer to previous list. 
    zoom_history* pNext;                    //Pointer to next list. 
}; 










/**********  CLASS DEFINITIONS **********/
//Note: Methods are defined later. 



class total_bounds_info
{   
    public: 
        //// Declare variables
        vector <bounds_info> BoundsHistory;                         //This vector stores every single 
                                                                    //group of bounds used. 
        long double Scale;                                          //Scale to draw in. 
        
        
        //// Declare methods
        total_bounds_info(long double, long double, long double, long double);          //Constructor
        void NewBounds(long double, long double, long double, long double);             //Adds a new group of bounds to BoundsHistory. 
        void BackBound();                                           //Goes back a bound. 
        void ClearBounds();                                         //Clears the bound history.  
} TotalBoundsInfo(2.0, -2.0, 1.0, -1.0); 

class drawing_info
{
    public: 
        //// Declare variables
        BYTE Hue;                                //Hue to draw in. 
        unsigned int Sat;                        //Saturation of the image. 
        BYTE Lum;                                //Luminosity of the image. 
        
        bool Invert;                             //Whether or not to invert the picture. 
        bool isMonochrome;                       //Whether or not to draw in monochrome. 
        unsigned short ThreadNo;                 //Number of threads to draw with. 
        unsigned short EscapeNo;                 //Number of iterations necessary to determine whether 
                                                 //or not a point is in the Mandelbrot set. 
        bool ProperMagnify;                      //Whether or not the image's escape number should
                                                 //be dynamic, determined by how zoomed in one is. 
         
        //// Declare methods. 
        drawing_info(BYTE, BYTE, BYTE, bool,     //Constructor
                     bool, unsigned short, 
                     unsigned short, bool); 
        void ResetMagnify();                     //For resetting the magnification; only relevant
                                                 //when ProperMagnify is true. 
} DrawingInfo(0, 128, 128, false, true, 2, 500, false);












/**********  FUNCTION DEFINITIONS **********/
//NOTE: This does not include procedures. 

void DrawMandelbrot(HDC hDc) //DC to draw to. 
{
    /*
        This function draws the Mandelbrot set to the given DC with the 
        drawing information in global variables. It draws using threads, vertically. 
        
        Note that here, we are only using one thread_info1 variable. We set its values 
        and execute a thread with a pointer to ThreadInfo. The thread then picks
        up that pointer, and sets ThreadInfo.FinishedInitiating to true. Then, the
        values of ThreadInfo are reset, and the process is repeated. We need to wait
        for FinishedInitiating to be true, in the interest of synchronization. If 
        it is false, the value in ThreadInfo's address will be changed too soon, 
        and the thread will pick up the changed, incorrect value. 
    */
    
    ////  Set the variables.     
    //Set the memory DC, and set its bitmap. 
    ::hDcMem = CreateCompatibleDC(hDc); 
    SelectObject(::hDcMem, CreateCompatibleBitmap(hDc, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT)); 
    
    //Set the thread informations' device context. 
    thread_info1 ThreadInfo; 
    ThreadInfo.hDc = hDc; 
    
    //Set the array of running threads and check for validity. 
    HANDLE* hThreads = new(nothrow) HANDLE[DrawingInfo.ThreadNo - 1]; 
    if (hThreads == NULL)
    {
        ShowError("Failed to allocate handle memory for threads."); 
        return; 
    }
    
    //For the loop
    float StartColumn = 0;                                         //Column to start drawing to.  
    float Increment = MAIN_WINDOW_WIDTH / DrawingInfo.ThreadNo;    //Columns to increment by. 
    
    // Note that these are floating values, to accomodate rounding errors. 
    
    
    //Assign each of the threads' values and create the threads. 
    for (unsigned short i = 0; i < DrawingInfo.ThreadNo; ++i)
    {
        //Replace the current columns in the thread. 
        ThreadInfo.StartColumn = int(StartColumn); 
        ThreadInfo.EndColumn = int(StartColumn + Increment); 
        ThreadInfo.FinishedInitiating = false; 
        
        //Update the current column. 
        StartColumn += Increment;
        
        //Create the thread. 
        hThreads[i] = CreateThread(NULL, 0, DrawColumn, &ThreadInfo, 0, NULL); 
        
        //Make sure it's valid. 
        if (hThreads[i] == NULL)
        {
            //Close all the threads, display failure, and exit. 
            for (unsigned short j = 0; j < i; ++j)
                CloseHandle(hThreads[j]); 
            ShowError("Failed to create a thread."); 
            return; 
        }
        
        //Wait for it to finish being created. 
        while (!ThreadInfo.FinishedInitiating); 
    }
    
    //Wait for the threads to finish drawing. 
    WaitForMultipleObjects(DrawingInfo.ThreadNo, hThreads, true, INFINITE); 
    
    //Blit the memory DC onto the screen. 
    BitBlt(hDc, 0, 0, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT, ::hDcMem, 0, 0, SRCCOPY); 
}









/**********  THREAD DEFINITIONS **********/
DWORD WINAPI DrawColumn(LPVOID param) //Handle to thread info. 
{
    /*
        Draws the Mandelbrot set in the given columns. To do this, it will
        draw on a temporary memory DC and then blit it onto the global 
        memory DC. 
    */
    ////  Set variables  
    //Get the thread information. 
    thread_info1* pThreadInfo = (thread_info1*)param; 
    
    //THE CONTENTS IN THIS POINTER ARE SUBJECT TO CHANGE, since pThreadInfo
    //points to a single variable whose values are constantly reset in DrawMandelbrot(). 
    //We must therefore copy each of the variables in pThreadInfo, where they are 
    //certain to not change. 
    int StartColumn = pThreadInfo->StartColumn; 
    int EndColumn = pThreadInfo->EndColumn; 
    HDC hDc = pThreadInfo->hDc; 
    
    //The temporary DC that will be drawn to. 
    HDC hDcTemp = CreateCompatibleDC(hDc); 
    SelectObject(hDcTemp, CreateCompatibleBitmap(hDc, EndColumn - StartColumn, 
                                                 MAIN_WINDOW_HEIGHT)); 
    
    //You have finished initiating. 
    pThreadInfo->FinishedInitiating = true;  
    
    //Original real and imaginary parts of the complex number. 
    long double OrRePart; 
    long double OrImPart; 
    
    //Current real and imaginary parts of the complex number. 
    long double RePart; 
    long double ImPart; 
    
    //Set the squares of the parts. 
    long double RePartSqr; 
    long double ImPartSqr; 
    
    //Saturation for coloring (dependent on escape speeds). 
    BYTE Sat; 
    
    //Counter for the escape loops. 
    unsigned short Count; 
    
    
    ////  Drawing. 
    for (unsigned short x = StartColumn; x < EndColumn; ++x)
    {
        for (unsigned short y = 0; y <= MAIN_WINDOW_HEIGHT; ++y)
        {
            //Set the real part and imaginary parts. 
            OrRePart = TotalBoundsInfo.BoundsHistory[TotalBoundsInfo.BoundsHistory.size() - 1].ReMin + 
                       x * TotalBoundsInfo.Scale; 
            OrImPart = TotalBoundsInfo.BoundsHistory[TotalBoundsInfo.BoundsHistory.size() - 1].ImMin +
                       y * TotalBoundsInfo.Scale; 
            
            RePart = OrRePart; 
            ImPart = OrImPart; 
            
            //Start the escape loop. 
            Count = 0; 
            while (Count < DrawingInfo.EscapeNo)
            {
                //Sets squares of the complex number. 
                RePartSqr = RePart * RePart; 
                ImPartSqr = ImPart * ImPart; 
                
                //Checks if its magnitude is greater than 2; if so, escape. 
                if (RePartSqr + ImPartSqr >= 4)
                    break; 
                
                //Update complex values. 
                ImPart = 2 * RePart * ImPart + OrImPart; 
                RePart = RePartSqr - ImPartSqr + OrRePart; 
                
                //Increase the counter. 
                ++Count; 
            }
            
            //Get the color and put the pixel down (in monochrome)
            Sat = 255 - int((Count * 255) / DrawingInfo.EscapeNo); 
            SetPixel(hDcTemp, x - StartColumn, y, RGB(Sat, Sat, Sat)); 
        }
    }
    
    //Blit the copy onto the memory DC. 
    BitBlt(::hDcMem, StartColumn, 0, EndColumn - StartColumn, MAIN_WINDOW_HEIGHT, 
           hDcTemp, 0, 0, SRCCOPY); 
    
    //Terminate the thread. 
    return 0; 
}











/**********  METHOD DEFINITIONS **********/

////  drawing_info
drawing_info::drawing_info(BYTE Hue0,                     //Hue to set
                           BYTE Sat0,                     //Saturation to set
                           BYTE Lum0,                     //Luminosity to set
                           bool Invert0,                  //State of inversion to set
                           bool isMonochrome0,            //Monochromity to set
                           unsigned short ThreadNo0,      //Number of threads to set
                           unsigned short EscapeNo0,      //Number of iterations to set
                           bool ProperMagnify0)           //State of proper magnification to set
{
    //Set all the variables. 
    Hue = Hue0; 
    Sat = Sat0; 
    Lum = Lum0; 
    Invert = Invert0; 
    isMonochrome = isMonochrome0; 
    ThreadNo = ThreadNo0; 
    EscapeNo = EscapeNo0; 
    ProperMagnify = ProperMagnify0; 
}

void drawing_info::ResetMagnify()
{
     /*
         This resets the drawing scale as you zoom in closer. 
     */
     
     //You're supposed to be properly magnified. 
     assert(DrawingInfo.ProperMagnify); 
}


//// total_bounds_info

void total_bounds_info::NewBounds(long double ReMax,   //New max. bound for the reals. 
                                  long double ReMin,   //New min. bound for the reals. 
                                  long double ImMax,   //New max. bound for the imaginaries. 
                                  long double ImMin)   //New min. bound for the imaginaries. 
{
    /*
        This method gives a new set of bounds to the history. 
    */
    
    //Transfer arguments to a new set of information about bounds. 
    bounds_info BoundsInfo; 
    BoundsInfo.ReMax = ReMax; 
    BoundsInfo.ReMin = ReMin; 
    BoundsInfo.ImMax = ImMax; 
    BoundsInfo.ImMin = ImMin; 
    
    //Add that to the history. 
    BoundsHistory.push_back(BoundsInfo); 
    
    //Reset the scale. 
    Scale = (ReMax - ReMin) / MAIN_WINDOW_WIDTH; 
}


total_bounds_info::total_bounds_info(long double ReMax,   //New max. bound for the reals. 
                                     long double ReMin,   //New min. bound for the reals. 
                                     long double ImMax,   //New max. bound for the imaginaries. 
                                     long double ImMin)   //New min. bound for the imaginaries. 
{
    /*
        This constructor adds the initial values given to the history. 
    */
    
    //Transfer arguments to a new set of information about bounds. 
    bounds_info BoundsInfo; 
    BoundsInfo.ReMax = ReMax; 
    BoundsInfo.ReMin = ReMin; 
    BoundsInfo.ImMax = ImMax; 
    BoundsInfo.ImMin = ImMin; 
    
    //Add that to the history. 
    BoundsHistory.push_back(BoundsInfo); 
    
    //Reset the scale. 
    Scale = (ReMax - ReMin) / MAIN_WINDOW_WIDTH; 
}
    











/**********  MAIN  **********/

int WINAPI WinMain(HINSTANCE hThisInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpszArgument,
                   int cmdShow)
{
    //Declare variables
    MSG msg;                  //Generic variable used to store/process messages. 
    WNDCLASSEX winclMain;     //Store the information for the window's class. 
    
    ////Set the window class's information
    //Basic window information. 
    winclMain.hInstance = hThisInstance;
    winclMain.lpszClassName = ::MainWinClassName;
    winclMain.lpfnWndProc = MainWinProc; 
    winclMain.style = CS_DBLCLKS; 
    winclMain.cbSize = sizeof(WNDCLASSEX);

    //Icon, menu, and cursor. 
    winclMain.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    winclMain.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    winclMain.hCursor = LoadCursor(NULL, IDC_ARROW);
    winclMain.lpszMenuName = NULL;
    
    //No extra memory needs to be allocated. 
    winclMain.cbClsExtra = 0;
    winclMain.cbWndExtra = 0;
    
    //Use Window's default background color as the background of the window. 
    winclMain.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
    
    //Register the class
    if (!RegisterClassEx(&winclMain))
    {
        ShowError("Failed to register class."); 
        return 0; 
    }
    
    //Create the window 
    ::hMainWindow = CreateWindowEx(0, 
                                   ::MainWinClassName,
                                   MAIN_WINDOW_TITLE_TEXT,
                                   WS_OVERLAPPEDWINDOW,
                                   MAIN_WINDOW_START_X, 
                                   MAIN_WINDOW_START_Y,
                                   MAIN_WINDOW_WIDTH,
                                   MAIN_WINDOW_HEIGHT + 16,
                                   HWND_DESKTOP,
                                   NULL,
                                   hThisInstance,
                                   NULL);
    
    
    //Show the window.
    ShowWindow(::hMainWindow, cmdShow);
    
    //Go through the message loop.
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    //Return what was posted by PostQuitMessage (since that will exit
    //the message loop)
    return msg.wParam;
}







 
/**********  PROCEDURE DEFINITIONS  **********/

LRESULT CALLBACK MainWinProc(HWND hWnd, 
                             UINT msg, 
                             WPARAM wParam, 
                             LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE: 
        {
            //Get the window's DC. 
            HDC hDc = GetDC(hWnd);
            
            //Draw the Mandelbrot set. 
            DrawMandelbrot(hDc);
            
            //Release the DC. 
            ReleaseDC(hWnd, hDc); 
        }
        break; 
        
        case WM_PAINT: 
        {
            //Set the paintstruct variable. 
            PAINTSTRUCT PaintStruct;  
            
            //Begin painting, blit the memory DC to the screen's DC, and end the painting. 
            HDC hDc = BeginPaint(hWnd, &PaintStruct); 
            BitBlt(hDc, 0, 0, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT, ::hDcMem, 0, 0, SRCCOPY); 
            EndPaint(hWnd, &PaintStruct); 
            
            //Release the DC. 
            ReleaseDC(hWnd, hDc); 
        }
        break; 
        
        case WM_DESTROY:
            PostQuitMessage(0); 
        break; 
        
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}
