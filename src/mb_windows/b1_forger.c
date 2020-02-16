///*
// * b1_forger.c
// *
// *  Created on: 8 груд. 2019 р.
// *      Author: KozakVF
// */
//
//
///*
// * b1_rend_rect.c
// *
// *  Created on: 13 серп. 2019 р.
// *      Author: KozakVF
// */
//
////https://www-user.tu-chemnitz.de/~heha/viewchm.php/hs/petzold.chm/petzoldi/ch20c.htm
///*------------------------------------------
//   RNDRCTMT.C -- Displays Random Rectangles
//                 (c) Charles Petzold, 1998
//  ------------------------------------------*/
////#include "multi.h"
////#ifdef RND_RECTANGLE
//#include <windows.h>
//#include <process.h>
//#include "stdint.h"
//LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;
//
//HWND hwnd ;
//int  cxClient, cyClient ;
//
//int WINAPI WinMain1 (HINSTANCE hInstance, HINSTANCE hPrevInstance,
//                    PSTR szCmdLine, int iCmdShow)
//{
//     static TCHAR szAppName[] = TEXT ("RndRctMT") ;
//     MSG          msg ;
//     WNDCLASS     wndclass ;
//
//     wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
//     wndclass.lpfnWndProc   = WndProc ;
//     wndclass.cbClsExtra    = 0 ;
//     wndclass.cbWndExtra    = 0 ;
//     wndclass.hInstance     = hInstance ;
//     wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
//     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
//     wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
//     wndclass.lpszMenuName  = NULL ;
//     wndclass.lpszClassName = szAppName ;
//
//     if (!RegisterClass (&wndclass))
//     {
//          MessageBox (NULL, TEXT ("This program requires Windows NT!"),
//                      szAppName, MB_ICONERROR) ;
//          return 0 ;
//     }
//
//     hwnd = CreateWindow (szAppName, TEXT ("Random Rectangles"),
//                          WS_OVERLAPPEDWINDOW,
//                          CW_USEDEFAULT, CW_USEDEFAULT,
//                          CW_USEDEFAULT, CW_USEDEFAULT,
//                          NULL, NULL, hInstance, NULL) ;
//
//     ShowWindow (hwnd, iCmdShow) ;
//     UpdateWindow (hwnd) ;
//
//     while (GetMessage (&msg, NULL, 0, 0))
//     {
//          TranslateMessage (&msg) ;
//          DispatchMessage (&msg) ;
//     }
//     return msg.wParam ;
//}
//
//VOID Thread (PVOID pvoid)
//{
//     HBRUSH hBrush ;
//     HDC    hdc ;
//     int    xLeft, xRight, yTop, yBottom, iRed, iGreen, iBlue ;
//
//     while (TRUE)
//     {
//          if (cxClient != 0 || cyClient != 0)
//          {
//               xLeft   = rand () % cxClient ;
//               xRight  = rand () % cxClient ;
//               yTop    = rand () % cyClient ;
//               yBottom = rand () % cyClient ;
//               iRed=0;
//               iRed    = rand () % 16;
//               iGreen  = rand () & 255 ;
//               iBlue   = rand () & 255 ;
//               uint64_t
//               hdc = GetDC (hwnd) ;
////               hBrush = CreateSolidBrush (RGB (iRed, iGreen, iBlue)) ;
//
//              // HBRUSH WINAPI yy = CreateBrushIndirect (const LOGBRUSH *)
////               SelectObject (hdc, hBrush) ;
//               RECT rect={0};
//                  rect.bottom=max (yTop, yBottom);
//			   rect.right=max(xLeft, xRight);
//			   rect.top=min (yTop, yBottom);
//			   rect.left=min (xLeft, xRight);
//			   uint64_t *hmyBrush = 0;
//			   uint64_t col=0x00ff00ff00000000;
//			   volatile uint64_t brush4=0;
//			   brush4=(HBRUSH)25;
//			   hmyBrush=&col;
//			   Sleep(500);
//			   iRed++;
//               int yy=FillRect(hdc,&rect,iRed /*(HBRUSH)( iRed )*/);
////               int yy=FillRect(hdc,&rect,hmyBrush /*(HBRUSH)( iRed )*/);
//
//
//               ReleaseDC (hwnd, hdc) ;
////               DeleteObject (hBrush) ;
//          }
//     }
//}
//LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//     switch (message)
//     {
//     case WM_CREATE:
//          _beginthread (Thread, 0, NULL) ;
//          return 0 ;
//
//     case WM_SIZE:
//          cxClient = LOWORD (lParam) ;
//          cyClient = HIWORD (lParam) ;
//          return 0 ;
//
//     case WM_DESTROY:
//          PostQuitMessage (0) ;
//          return 0 ;
//     }
//     return DefWindowProc (hwnd, message, wParam, lParam) ;
//}
////#endif
