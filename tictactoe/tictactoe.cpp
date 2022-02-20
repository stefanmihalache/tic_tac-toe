// tictactoe.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "tictactoe.h"
#include <windowsx.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TICTACTOE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TICTACTOE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TICTACTOE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.hbrBackground = (HBRUSH)(GetStockObject(GRAY_BRUSH));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TICTACTOE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
// Global variables: 
const int CELL_SIZE = 100;
HBRUSH hbr1, hbr2;
int playerTurn = 1;
int gameBoard[9] = { 0,0,0 ,0,0,0 ,0,0,0 };
int winner = 0;
int wins[3];


BOOL GetGameBoardRect(HWND hWnd, RECT* pRect)
{
    RECT rc;
    if (GetClientRect(hWnd, &rc))
    {
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        pRect->left = (width - CELL_SIZE * 3) / 2;
        pRect->top = (height - CELL_SIZE * 3) / 2;

        pRect->right = pRect->left + CELL_SIZE * 3;
        pRect-> bottom = pRect->top + CELL_SIZE * 3;
        return TRUE;
    }

    SetRectEmpty(pRect);
    return FALSE;
}

void DrawLine(HDC hdc, int x1, int y1, int x2, int y2)
{
    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);
}
int GetCellNumberFromPoint(HWND hWnd, int x, int y)
{
    RECT rc;
    POINT pt = {x,y};

    if (GetGameBoardRect(hWnd, &rc))
    {
        if (PtInRect(&rc, pt))
        {
            // user clicked inside game board
            x = pt.x - rc.left;
            y = pt.y - rc.top;

            int column = x / CELL_SIZE;
            int row = y / CELL_SIZE;

            return column + row * 3;
        }
    }
    return -1; //outside game board or faliure
}
BOOL GetCellRect(HWND hWnd, int index, RECT *pRect)
{
    RECT rcBoard;
    SetRectEmpty(pRect);
    if (index < 0)
        return FALSE;

    if (GetGameBoardRect(hWnd, &rcBoard))
    {
        int y = index / 3;
        int x = index % 3;

        pRect->left = rcBoard.left + x * CELL_SIZE+1;
        pRect->top = rcBoard.top + y * CELL_SIZE+1;

        pRect->right = pRect->left + CELL_SIZE - 1 ;
        pRect->bottom = pRect->top + CELL_SIZE - 1;

        return TRUE;
    }
    return FALSE;
}
/*
* Returns:
* 0 - No winner
* 1 - Player 1 wins
* 2 - Player 2 wins
* 3 - It's a draw
*/
int GetWinner(int wins[3])
{
    int cells[] = { 0,1,2, 3,4,5, 6,7,8,  0,3,6 ,1,4,7, 2,5,8,  0,4,8, 2,4,6 };

    //check for winner
    for (int i = 0; i < ARRAYSIZE(cells); i += 3)
        if (gameBoard[cells[i]]!=0 && gameBoard[cells[i]] == gameBoard[cells[i + 1]] && gameBoard[cells[i]] == gameBoard[cells[i + 2]])
        {
            wins[0] = gameBoard[cells[i]];
            wins[1] = gameBoard[cells[i+1]];
            wins[3] = gameBoard[cells[i + 2]];

            return gameBoard[cells[i]];
        }
    for (int i = 0; i < ARRAYSIZE(gameBoard); i++)
        if (gameBoard[i] == 0)
            return 0;

    return 3;
}

void ShowTurn(HWND hWnd, HDC hdc)
{
    RECT rc;
    static const WCHAR szTurn1[] = L"Turn: Player 1";
    static const WCHAR szTurn2[] = L"Turn: Player 2";

    const WCHAR* psTurnText = NULL;

    switch (winner)
    {
    case 0:
        psTurnText = (playerTurn == 1) ? szTurn1 : szTurn2;
        break;
    case 1:
        psTurnText = L"Player 1 is the winner!";
        break;
    case 2:
        psTurnText = L"Player 2 is the winner!";
        break;
    case 3:
        psTurnText = L"It's a draw";
        break;
    }

    if (GetClientRect(hWnd, &rc) && psTurnText !=NULL)
    {   
        rc.top = rc.bottom - 48;

        FillRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        
        DrawText(hdc, psTurnText, lstrlen(psTurnText), &rc, DT_CENTER);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        hbr1 = CreateSolidBrush(RGB(255, 0, 0));
        hbr2 = CreateSolidBrush(RGB(0, 0, 255));
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case ID_FILE_NEWGAME:
            {
                int ret =MessageBox(hWnd, L"Are you sure you want to start a new game?", L"New Game", MB_YESNO | MB_ICONQUESTION);
                if (IDYES == ret)
                {
                    //reset and start a new game
                    playerTurn = 1;
                    winner = 0;
                    ZeroMemory(gameBoard, sizeof(gameBoard));

                    InvalidateRect(hWnd, NULL, TRUE);
                    UpdateWindow(hWnd);
                }
            }
            break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_LBUTTONDOWN:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        if (playerTurn == 0)
            break;

        int index = GetCellNumberFromPoint(hWnd, xPos, yPos);

        HDC hdc = GetDC(hWnd);
        if (hdc != NULL)
        {
            

            if (index != -1)
            {   
                RECT rcCell;
                if ((gameBoard[index] == 0) && GetCellRect(hWnd, index, &rcCell))
                {   
                    gameBoard[index] = playerTurn;
                    FillRect(hdc, &rcCell,(playerTurn ==2) ? hbr2 : hbr1 );
                    
                    //check for winner
                    winner = GetWinner(wins);
                    if (winner == 1 || winner == 2)
                    {
                        MessageBox(hWnd, (winner == 1) ? L"Player 1 is the winner!" : L"Player 2 is the winner!",L"You win!", MB_OK | MB_ICONINFORMATION);
                        playerTurn = 0;
                    }
                    else if (winner == 3)
                    {
                        MessageBox(hWnd, L"Oh! No one wins this time.", L"It's a draw!", MB_OK | MB_ICONINFORMATION);
                        playerTurn = 0;
                    }
                    else if( winner ==  0)
                        playerTurn = playerTurn % 2 + 1;
                }

                //Display turn
                ShowTurn(hWnd, hdc);
            }
        }

        ReleaseDC(hWnd,hdc);
    }
    break;
    case WM_GETMINMAXINFO:
        {
            MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;
            
            pMinMax->ptMinTrackSize.x = CELL_SIZE * 5;
            pMinMax->ptMinTrackSize.y = CELL_SIZE * 5;
            pMinMax->ptMaxTrackSize.x = CELL_SIZE * 7;
            pMinMax->ptMaxTrackSize.y = CELL_SIZE * 7;
        }
        break;
    case WM_PAINT:
        {
           
            PAINTSTRUCT ps;
             HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            RECT rc;
            if (GetGameBoardRect(hWnd, &rc))
            {   
                RECT rcClient;
                    
                //Display player text and turn

                if (GetClientRect(hWnd, &rcClient))
                {   
                    const WCHAR szPlayer1 [] = L"Player 1";
                    const WCHAR szPlayer2 [] = L"Player 2";

                    SetBkMode(hdc, TRANSPARENT);

                    //draw player 1 and player 2 text
                    SetTextColor(hdc, RGB(255, 255, 0));
                    TextOut(hdc, 16, 16, szPlayer1 , lstrlen(szPlayer1));
                    SetTextColor(hdc, RGB(0, 0, 255));
                    TextOut(hdc, rcClient.right -72, 16, szPlayer2 , lstrlen(szPlayer2));

                    //Display turn
                    ShowTurn(hWnd, hdc);
                }

                

                Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
                //FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
                //  
                //Draw vertical lines
                DrawLine(hdc, rc.left + 100, rc.top, rc.left + 100, rc.top + 300);
                DrawLine(hdc, rc.left + 200, rc.top, rc.left + 200, rc.top + 300);
                //Draw horizontal lines
                DrawLine(hdc, rc.left, rc.top+100, rc.left + 300, rc.top + 100);
                DrawLine(hdc, rc.left, rc.top+200, rc.left + 300, rc.top + 200);
                
                RECT rcCell;

                for (int i = 0; i < 9; i++)
                    if ((gameBoard[i] != 0) && GetCellRect(hWnd, i, &rcCell))
                        FillRect(hdc, &rcCell, (gameBoard[i] == 2) ? hbr2 : hbr1);

            }
                
            
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
    {
        DeleteObject(hbr1);
        DeleteObject(hbr2);
        PostQuitMessage(0);
    }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
