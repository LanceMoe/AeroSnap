#include "AeroSnap.h"
#include "Util.h"

wchar_t tzPath[MAX_PATH];//文件路径
wchar_t tzIniPath[MAX_PATH];//配置文件路径
wchar_t tzSavePath[MAX_PATH];//保存路径

int bSave;
bool bCutt;
bool bCurs;
bool bGrid;

int sequence = 1;

void AppendRadioItem(HMENU hMenu,int id,int check,wchar_t *name)
{
	MENUITEMINFO MenuInfo = {0};  
	MenuInfo.cbSize = sizeof(MENUITEMINFO);
	MenuInfo.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING | MIIM_STATE;
	MenuInfo.fType = MFT_STRING | MFT_RADIOCHECK;  
	MenuInfo.fState = MFS_ENABLED | ((check==1||check==2)?MFS_CHECKED:0) | ((check==3)?MFS_DISABLED:0);
	MenuInfo.wID = id;  
	MenuInfo.dwTypeData = name;  

	InsertMenuItem(hMenu,id,TRUE,&MenuInfo);
}

void DealWithMenu(HWND hWnd,int id)
{
	switch(id)
	{
	case 1000:
		Sleep(3*1000);
		SendMessage(hWnd,WM_HOTKEY,1000,0);
		break;
	case 1002:
		bSave = 1;
		SetPrivateProfileInt(L"程序设置", L"保存方式", bSave, tzIniPath);
		break;
	case 1003:
		bSave = 2;
		SetPrivateProfileInt(L"程序设置", L"保存方式", bSave, tzIniPath);
		break;
	case 1009:
		bSave = 3;
		SetPrivateProfileInt(L"程序设置", L"保存方式", bSave, tzIniPath);
		break;

	case 1004:
		bCutt = !bCutt;
		SetPrivateProfileInt(L"程序设置", L"自动裁剪", bCutt, tzIniPath);
		break;
	case 1005:
		bCurs = !bCurs;
		SetPrivateProfileInt(L"程序设置", L"包含指针", bCurs, tzIniPath);
		break;
	case 1006:
		bGrid = !bGrid;
		SetPrivateProfileInt(L"程序设置", L"方格透明", bGrid, tzIniPath);
		break;

	case 1007:
		{
			static int run = false;
			if(run==false)
			{
				run=true;
				MessageBox(hWnd,L"AeroSnap 1.11 - 2013.01.21\n\n本工具专用于对透明窗口截图（保留透明通道）。\n开发： http://www.shuax.com",szTitle,MB_OK);
				run=false;
			}

		}
		break;
	case 1008:
		DestroyWindow(hWnd);
		break;
	}
}
void ShowContextMenu(HWND hWnd)
{
	POINT pt;
	GetCursorPos(&pt);

	HMENU hMenu, hPopMenu;
	hMenu = CreatePopupMenu();
	
	//hPopMenu = CreatePopupMenu();
	//AppendRadioItem(hPopMenu,1001,2,L"自动截图");
	//AppendRadioItem(hPopMenu,1000,3,L"手动截图");
	//AppendMenu(hMenu, MF_POPUP | MF_BYPOSITION, (UINT)hPopMenu, _T("截图方式"));


	AppendMenu(hMenu, MF_BYPOSITION, 1000, _T("开始截图\t\n延迟3秒"));
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

	hPopMenu = CreatePopupMenu();
	AppendRadioItem(hPopMenu,1002,bSave==1,L"自动保存");
	AppendRadioItem(hPopMenu,1003,bSave==2,L"手动保存");
	AppendRadioItem(hPopMenu,1009,bSave==3,L"复制到粘贴板");
	AppendMenu(hMenu, MF_POPUP | MF_BYPOSITION, (UINT)hPopMenu, _T("保存方式"));

	hPopMenu = CreatePopupMenu();
	AppendMenu(hPopMenu, MF_BYPOSITION | (bCutt?MF_CHECKED:0), 1004, _T("自动裁剪"));
	AppendMenu(hPopMenu, MF_BYPOSITION | (bCurs?MF_CHECKED:0), 1005, _T("包含指针"));
	AppendMenu(hPopMenu, MF_BYPOSITION | (bGrid?MF_CHECKED:0), 1006, _T("方格透明"));
	AppendMenu(hMenu, MF_POPUP | MF_BYPOSITION, (UINT)hPopMenu, _T("其它设置"));

	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_BYPOSITION, 1007, _T("关于我们"));
	AppendMenu(hMenu, MF_BYPOSITION, 1008, _T("退出程序"));

	SetForegroundWindow(hWnd);
	int id = TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL );
	DestroyMenu(hMenu);

	DealWithMenu(hWnd, id);
}
/*
void CenterWindow(HWND hWnd,int Weigth,int Hight)
{
	DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
	DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);

	RECT r = {0,0,Weigth,Hight};
	AdjustWindowRectEx(&r,dwStyle,0,dwExStyle);

	RECT w;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &w, 0);
	int width = r.right - r.left;
	int height = r.bottom - r.top;
	int x = ((w.right - w.left) / 2) - (width / 2);
	int y = ((w.bottom - w.top) / 2) - (height / 2);

	SetWindowPos(hWnd, 0, x, y, width, height, SWP_FRAMECHANGED | SWP_NOACTIVATE |SWP_NOOWNERZORDER);
}
*/

void BuildTrayIcon(HWND hWnd)
{
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = 1;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	nid.uCallbackMessage = WM_USER;
	nid.hIcon = LoadIcon(hInst, L"APPICON16");
	_tcscpy(nid.szTip, szTitle);
	nid.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;
	nid.uVersion = NOTIFYICON_VERSION;
	_tcscpy(nid.szInfoTitle, szTitle);
	Shell_NotifyIcon(NIM_ADD, &nid);
}

inline void SetTopMost(HWND hWnd,bool enable)
{
	if(enable) ::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	else ::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}
BOOL CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
			ShowWindow(hWnd,SW_HIDE);
			BuildTrayIcon(hWnd);

			SetWindowText(hWnd,szTitle);
			//CenterWindow(hWnd,512,256);

			OnRegisterHotKey(hWnd, tzIniPath);
			break;
		case WM_USER:
			switch (lParam)
			{
			case WM_RBUTTONDOWN:ShowContextMenu(hWnd);break;
			}
			break;
		case WM_HOTKEY:
			{
				static int run = 0;
				if(run==1) break;
				run=1;

				RECT rcRect = {0};

				HWND camera = GetForegroundWindow();
				SetForegroundWindow(camera);

				//if(!IsWindowVisible(camera)) break;
				GetWindowRect(camera,&rcRect);
				InflateRect(&rcRect,15,15);

				rcRect.left = max(rcRect.left,0);
				rcRect.right = min(rcRect.right,GetSystemMetrics(SM_CXSCREEN));
				rcRect.top = max(rcRect.top,0);
				rcRect.bottom = min(rcRect.bottom,GetSystemMetrics(SM_CYSCREEN));

				BLENDFUNCTION m_Blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
				POINT ptWinPos;
				SIZE sizeWindow;
				POINT ptSrc = {0, 0};
				ptWinPos.x = rcRect.left;
				ptWinPos.y = rcRect.top;
				sizeWindow.cx = rcRect.right-rcRect.left;
				sizeWindow.cy = rcRect.bottom-rcRect.top;

				DWORD *data_black;
				DWORD *data_white;
				HBITMAP black = SCreateCompatibleBitmap(sizeWindow.cx,sizeWindow.cy,(void**)&data_black,true,0xFF000000);
				HBITMAP white = SCreateCompatibleBitmap(sizeWindow.cx,sizeWindow.cy,(void**)&data_white,true,0xFFFFFFFF);


				HDC display = GetDC(NULL);//CreateDC(_T("DISPLAY"), NULL, NULL, NULL); 
				HDC hdc = GetDC(hWnd);
				HDC mdc1 = CreateCompatibleDC(hdc);
				HDC mdc2 = CreateCompatibleDC(hdc);

				SelectObject(mdc1, black);
				ShowWindow(hWnd,SW_SHOW);
				UpdateLayeredWindow(hWnd, hdc, &ptWinPos, &sizeWindow, mdc1, &ptSrc, 0, &m_Blend, ULW_ALPHA);
				SetTopMost(hWnd,true);
				SetTopMost(hWnd,false);
				SetForegroundWindow(camera);
				SetTopMost(camera,true);
				SetTopMost(camera,false);
				BitBlt(mdc1,0,0, sizeWindow.cx,sizeWindow.cy,display, ptWinPos.x, ptWinPos.y, SRCCOPY | CAPTUREBLT);
				ShowWindow(hWnd,SW_HIDE);

				//Sleep(100);
				SelectObject(mdc2, white);
				ShowWindow(hWnd,SW_SHOW);
				UpdateLayeredWindow(hWnd, hdc, &ptWinPos, &sizeWindow, mdc2, &ptSrc, 0, &m_Blend, ULW_ALPHA);
				SetTopMost(hWnd,true);
				SetTopMost(hWnd,false);
				SetForegroundWindow(camera);
				SetTopMost(camera,true);
				SetTopMost(camera,false);
				BitBlt(mdc2,0,0, sizeWindow.cx,sizeWindow.cy,display, ptWinPos.x, ptWinPos.y, SRCCOPY | CAPTUREBLT);
				ShowWindow(hWnd,SW_HIDE);

				MakeAlpha(data_black,&data_white,sizeWindow.cx*sizeWindow.cy);
				
				if(bCurs)
				{
                    //绘制鼠标
                    DrawCursor(mdc2,rcRect,ptWinPos);
				}

				if(bCutt)
				{
					//自动裁剪
					CuttBitmap(&data_white,sizeWindow.cx,sizeWindow.cy);
				}

				if(bGrid)
				{
					//透明方格
					MakeGrid(&data_white,sizeWindow.cx,sizeWindow.cy);
				}

				//保存文件
				wchar_t SavePath[MAX_PATH+1] = {0};
				if(bSave==2)
				{
					wsprintf(SavePath,_T("AeroSnap截图%d.png"), sequence);
					OPENFILENAME ofn = {0};
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hWnd;
					ofn.lpstrFile = SavePath;
					ofn.lpstrInitialDir = tzSavePath;
					ofn.lpstrFilter = L"PNG格式\0*.png\0";
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrDefExt = L"png";
					ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT;
					if( GetSaveFileName(&ofn) )
					{
						wcscpy(tzSavePath, SavePath);
						*(wcsrchr(tzSavePath, '\\')) = 0;

						WritePrivateProfileString(L"程序设置", _T("保存位置"), tzSavePath, tzIniPath);

						Save2PNG(SavePath,data_white,sizeWindow.cx,sizeWindow.cy);
						sequence++;
					}
				}
				else
				{
					if(bSave==1)
					{
						wchar_t Title[MAX_PATH+1] = {0};
						GetWindowText(camera,Title,MAX_PATH);

						if(Title[0]==0)
						{
							SYSTEMTIME SystemTime;
							GetLocalTime(&SystemTime);
							wsprintf(Title,_T("%04d%02d%02d%02d%02d%02d%03d"), SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
							wsprintf(SavePath,_T("%s\\%s.png"), tzSavePath, Title);
						}
						else
						{
							PathCleanupSpec(0,Title);
							wsprintf(SavePath,_T("%s\\%s.png"), tzSavePath, Title);
						}

						Save2PNG(SavePath,data_white,sizeWindow.cx,sizeWindow.cy);
					}
					else
					{
						//白色背景
						if(!bGrid) FillWhite(&data_white,sizeWindow.cx,sizeWindow.cy);

						BITMAPINFOHEADER   bi;  
						bi.biSize = sizeof(BITMAPINFOHEADER);      
						bi.biWidth = sizeWindow.cx;      
						bi.biHeight = sizeWindow.cy;    
						bi.biPlanes = 1;      
						bi.biBitCount = 32;      
						bi.biCompression = BI_RGB;
						bi.biXPelsPerMeter = 0;      
						bi.biYPelsPerMeter = 0;      
						bi.biClrUsed = 0;      
						bi.biClrImportant = 0;  
						bi.biSizeImage = sizeWindow.cx * sizeWindow.cy * 4;

						HANDLE hClip = GlobalAlloc(GHND,bi.biSizeImage + bi.biSize);  
						char *lpbitmap = (char *)GlobalLock(hClip);   
						memcpy(lpbitmap, &bi, bi.biSize);

						for(int i=bi.biHeight;i>0;i--)
						{
							memcpy(lpbitmap + bi.biSize + (bi.biHeight-i) * 4 * bi.biWidth, (char*)data_white + (i-1) * 4 * bi.biWidth, 4 * bi.biWidth);
						}
						GlobalUnlock(hClip);  


						OpenClipboard(NULL);  
						EmptyClipboard();   
						SetClipboardData(CF_DIB, hClip);
						CloseClipboard();
						//BITMAP bmp;  
						//GetObject(white, sizeof(BITMAP), &bmp);  //If hgdiobj is a handle to a bitmap   
						//created not by CreateDIBSection(),   
						//GetObject returns only the width, height, and color format information of the bitmap.  
						/*
						BITMAPINFOHEADER   bi;  
						bi.biSize = sizeof(BITMAPINFOHEADER);      
						bi.biWidth = sizeWindow.cx;      
						bi.biHeight = sizeWindow.cy;    
						bi.biPlanes = 1;      
						bi.biBitCount = 32;      
						bi.biCompression = BI_RGB;      
						bi.biSizeImage = 0;    
						bi.biXPelsPerMeter = 0;      
						bi.biYPelsPerMeter = 0;      
						bi.biClrUsed = 0;      
						bi.biClrImportant = 0;  
*/
						/*
						BITMAPV5HEADER rgbBitmapInfo;
						memset(&rgbBitmapInfo, 0, sizeof(BITMAPV5HEADER));
						rgbBitmapInfo.bV5Size = sizeof(BITMAPV5HEADER);
						rgbBitmapInfo.bV5Width = sizeWindow.cx;
						rgbBitmapInfo.bV5Height = sizeWindow.cy;
						rgbBitmapInfo.bV5Planes = 1;
						rgbBitmapInfo.bV5BitCount = 32;
						rgbBitmapInfo.bV5Compression = 0;
						rgbBitmapInfo.bV5RedMask =   0x00ff0000;
						rgbBitmapInfo.bV5GreenMask = 0x0000ff00;
						rgbBitmapInfo.bV5BlueMask =  0x000000ff;
						rgbBitmapInfo.bV5AlphaMask = 0xff000000;
						rgbBitmapInfo.bV5SizeImage = rgbBitmapInfo.bV5Height * rgbBitmapInfo.bV5Width * 4;
						
						rgbBitmapInfo.bV5ClrUsed = 0;
						rgbBitmapInfo.bV5ClrImportant = 0;
						((char *) &rgbBitmapInfo.bV5CSType)[3] = 's';
						((char *) &rgbBitmapInfo.bV5CSType)[2] = 'R';
						((char *) &rgbBitmapInfo.bV5CSType)[1] = 'G';
						((char *) &rgbBitmapInfo.bV5CSType)[0] = 'B';
						rgbBitmapInfo.bV5Intent = LCS_GM_GRAPHICS;
						rgbBitmapInfo.bV5Reserved = 0;
						

						//DWORD bmpSize = sizeWindow.cx *4 * sizeWindow.cy; //((sizeWindow.cx * bih.bV5BitCount + 31) / 32) * 4 * sizeWindow.cy;
						
						HANDLE hClip = GlobalAlloc(GHND,rgbBitmapInfo.bV5SizeImage + rgbBitmapInfo.bV5Size);  
						char *lpbitmap = (char *)GlobalLock(hClip);   
						memcpy(lpbitmap, &rgbBitmapInfo, rgbBitmapInfo.bV5Size);

						for(int i=rgbBitmapInfo.bV5Height;i>0;i--)
						{
							memcpy(lpbitmap + rgbBitmapInfo.bV5Size + (rgbBitmapInfo.bV5Height-i) * 4 * rgbBitmapInfo.bV5Width, (char*)data_white + i * 4 * rgbBitmapInfo.bV5Width, 4 * rgbBitmapInfo.bV5Width);
						}

						//memcpy(lpbitmap + rgbBitmapInfo.bV5Size, data_white, rgbBitmapInfo.bV5SizeImage);
						
						//GetDIBits(hdc, white, 0, (UINT)bmp.bmHeight, lpbitmap+sizeof(BITMAPINFO),(BITMAPINFO*)&bi, DIB_RGB_COLORS);  
						GlobalUnlock(hClip);  

						OpenClipboard(NULL);  
						EmptyClipboard();   
						SetClipboardData(CF_DIBV5, hClip);
						CloseClipboard();
						*/
					}
				}
				
				DeleteObject(white);
				DeleteObject(black);
				DeleteDC(mdc1);
				DeleteDC(mdc2);
				ReleaseDC(hWnd,hdc);
				ReleaseDC(NULL,display);

				run=0;
			}
			break;
		case WM_DESTROY:
			nid.hWnd = hWnd;
			nid.uID = 1;
			Shell_NotifyIcon(NIM_DELETE, &nid);
			GdiplusShutdown(gdiplusToken);
			PostQuitMessage(0);
			return TRUE;
		default:
			if (uMsg == RegisterWindowMessage(L"TaskbarCreated"))
			{
				BuildTrayIcon(hWnd);
			}
			break;
	}
	return FALSE;
}
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	hInst = hInstance;
	HANDLE hMutex = CreateMutex(NULL , TRUE , L"AeroSnapSingleInstance");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hMutex);
		return 0;
	}

	GetModuleFileName(hInstance, tzPath, sizeof(tzPath) / sizeof(TCHAR));
	*(wcsrchr(tzPath, '\\')) = 0;
	
	wcscpy(tzIniPath,tzPath);
	wcscat(tzIniPath,L"\\AeroSnap.ini");

	GetPrivateProfileString(L"程序设置", _T("保存位置"), L"", tzSavePath, MAX_PATH, tzIniPath);
	if(tzSavePath[0]==0)
	{
		wcscpy(tzSavePath, tzPath);
		WritePrivateProfileString(L"程序设置", _T("保存位置"), tzSavePath, tzIniPath);
	}

	bSave =  GetPrivateProfileInt(L"程序设置", L"保存方式", 1, tzIniPath);
	bCutt =  GetPrivateProfileInt(L"程序设置", L"自动裁剪", 1, tzIniPath);
	bCurs =  GetPrivateProfileInt(L"程序设置", L"包含指针", 0, tzIniPath);
	bGrid =  GetPrivateProfileInt(L"程序设置", L"方格透明", 0, tzIniPath);


	//InitCommonControls();
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken,&gdiplusStartupInput,NULL);
	
	return DialogBox(hInstance, MAKEINTRESOURCE(100), NULL, (DLGPROC)DialogProc);
}