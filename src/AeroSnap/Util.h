#define GDIPVER 0x0110
#include <objbase.h> 
#include <gdiplus.h>
#include <GdiplusFlat.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")
ULONG_PTR  gdiplusToken;

#include "mmx.h"

void SetPrivateProfileInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,INT nDefault,LPCTSTR lpFileName)
{
	TCHAR buffer[1024];
	wsprintf(buffer,_T("%d"),nDefault);
	WritePrivateProfileString(lpAppName,lpKeyName, buffer, lpFileName);
}
int   GetEncoderClsid(const   WCHAR*   format,   CLSID*   pClsid) 
{ 
	UINT     num   =   0;                     //   number   of   image   encoders 
	UINT     size   =   0;                   //   size   of   the   image   encoder   array   in   bytes 

	ImageCodecInfo*   pImageCodecInfo   =   NULL; 

	GetImageEncodersSize(&num,   &size); 
	if(size   ==   0) 
		return   -1;     //   Failure 

	pImageCodecInfo   =   (ImageCodecInfo*)(malloc(size)); 
	if(pImageCodecInfo   ==   NULL) 
		return   -1;     //   Failure 

	GetImageEncoders(num,   size,   pImageCodecInfo); 

	for(UINT   j   =   0;   j   <   num;   ++j) 
	{ 
		if(   wcscmp(pImageCodecInfo[j].MimeType,   format)   ==   0   ) 
		{ 
			*pClsid   =   pImageCodecInfo[j].Clsid; 
			free(pImageCodecInfo); 
			return   j;     //   Success 
		}         
	} 

	free(pImageCodecInfo); 
	return   -1;     //   Failure 
}

HBITMAP SCreateCompatibleBitmap(int Width,int Height,void **data,bool NeedFill,DWORD filled)
{
	void					  *pBits;
	BITMAPINFOHEADER          bmih;   
	ZeroMemory( &bmih, sizeof(BITMAPINFOHEADER));   

	bmih.biSize                 = sizeof (BITMAPINFOHEADER) ;   
	bmih.biWidth                = Width;   
	bmih.biHeight               = -Height;   
	bmih.biPlanes               = 1 ;   
	bmih.biBitCount             = 32;        //这里一定要是32   
	bmih.biCompression          = BI_RGB ;   
	bmih.biSizeImage            = 0 ;   
	bmih.biXPelsPerMeter        = 0 ;   
	bmih.biYPelsPerMeter        = 0 ;   
	bmih.biClrUsed              = 0 ;   
	bmih.biClrImportant         = 0 ;   

	HBITMAP hBitMap = CreateDIBSection (NULL, (BITMAPINFO *)  &bmih, DIB_RGB_COLORS, &pBits, NULL, 0) ;  

	*data = pBits;
	
	if(NeedFill)
	{
        DWORD *fill = (DWORD*)pBits;
        for(int i=0;i<Width*Height;i++)
        {
            fill[i] = filled;
        }
	}
	
	return hBitMap;  
}
bool IsSystemWin7()
{
	static int bWin7 = -1;

	if(bWin7==-1)
	{
		OSVERSIONINFO osvi = {0}; 
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		::GetVersionEx(&osvi); 
		if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&    
			osvi.dwMajorVersion == 6 &&   
			osvi.dwMinorVersion == 1 ) 
		{
			bWin7 = true; 
		}
		else bWin7 = false;

	}

	return bWin7!=0; 
}

void MakeAlpha(DWORD *data_black,DWORD **data,int length)
{
	DWORD *data_white = *data;
	for(int i=0;i<length;i++)
	{
		BYTE *color1 = (BYTE*)&data_black[i];
		BYTE *color2 = (BYTE*)&data_white[i];

		int alpha1 = 255 + color1[0] - color2[0];
		int alpha2 = 255 + color1[1] - color2[1];
		int alpha3 = 255 + color1[2] - color2[2];
		
		int alpha = min(255,max(alpha1,max(alpha2,alpha3)));
		
		if(alpha)
		{
			color2[0] = 255*color1[0]/alpha;
			color2[1] = 255*color1[1]/alpha;
			color2[2] = 255*color1[2]/alpha;
		}
		else
		{
			data_white[i] = 0;
		}
		color2[3] = alpha;
	}
}


void MakeGrid(DWORD **data,long Width,long Height)
{
	DWORD *data_white = *data;

	DWORD white = 0xFFFFFFFF;
	DWORD gray  = 0xFFCCCCCC;

	for(int h = 0;h<Height;h++)
	{
		DWORD color = 0;
		if((h+16)%16/8==0)
		{
			color = white;
		}
		else
		{
			color = gray;
		}

		for(int w = 0;w<Width;w++)
		{
			DWORD mixcolor = 0;
			if((w+16)%16/8==0)
			{
				mixcolor = color;
			}
			else
			{
				if(color==white) mixcolor = gray;
				else mixcolor = white;
			}
			//alpha混合
			data_white[ h*Width + w] = MMXAlphaMixed(data_white[ h*Width + w],mixcolor);
		}
	}
	__asm
	{
        emms
	}
}

void FillWhite(DWORD **data,long Width,long Height)
{
	DWORD *data_white = *data;

	for(int h = 0;h<Height;h++)
	{
		for(int w = 0;w<Width;w++)
		{
			//alpha混合
			data_white[ h*Width + w] = MMXAlphaMixed(data_white[ h*Width + w], 0xFFFFFFFF);
		}
	}
	__asm
	{
        emms
	}
}

void DrawCursor(HDC hdc,RECT rc,POINT pt)
{
    //鼠标指针
    CURSORINFO ci = {0};
    ci.cbSize = sizeof(CURSORINFO);
    if(GetCursorInfo(&ci))
    {
        //DbgPrint(L"GetCursorInfo");
        if(ci.flags==CURSOR_SHOWING && PtInRect(&rc,ci.ptScreenPos))
        {
            ICONINFO ii = {0};
            if(::GetIconInfo(ci.hCursor, &ii))
            {
                //DbgPrint(L"GetIconInfo");

                BITMAP bitmap = {0};
                if(::GetObject(ii.hbmMask,sizeof(BITMAP),&bitmap))
                {
					bool bBWCursor = false;
					if(ii.hbmColor==NULL)
					{
						bBWCursor = TRUE;
						bitmap.bmHeight /= 2;
					}
					BLENDFUNCTION m_Blend = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA};

                    HDC iconDC = CreateCompatibleDC(hdc);
                    DWORD *data_icon;
                    HBITMAP icon = SCreateCompatibleBitmap(bitmap.bmWidth,bitmap.bmHeight,(void**)&data_icon,false,0x00000000);
                    
                    SelectObject(iconDC,icon);
                    

					int offsetX = ci.ptScreenPos.x - pt.x - ii.xHotspot;
					int offsetY = ci.ptScreenPos.y - pt.y - ii.yHotspot;

					//BitBlt(iconDC,0,0,bitmap.bmWidth,bitmap.bmHeight,hdc,offsetX, offsetY,SRCCOPY | CAPTUREBLT);

					//DrawIconEx(iconDC,0,0,ci.hCursor,0,0,0,0,bBWCursor?DI_IMAGE:DI_NORMAL);
					
					//DbgPrint(L"%d,%d,%d,%d",bitmap.bmPlanes , bitmap.bmWidth , bitmap.bmHeight , bitmap.bmBitsPixel);
					/*
					DWORD dwSize = ::GetBitmapBits(ii.hbmMask,0,NULL);
					BYTE* pBits = (BYTE*)malloc(dwSize);
					GetBitmapBits(ii.hbmMask, dwSize, pBits);
					DWORD *pixs = (DWORD *)pBits;
					for(int i=0;i<dwSize/4;i++)
					{
						DbgPrint(L"%08X",pixs[i]);
					}
					free(pBits);
					*/
					//SetBitmapBits(icon,dwSize, pBits);
                    DrawIcon(iconDC,0,0,ci.hCursor);
                    //DrawIconEx(iconDC,0,0,ci.hCursor,0,0,0,0,DI_MASK);
					
					if(bBWCursor)
					{
						for(int i=0;i<bitmap.bmWidth*bitmap.bmHeight;i++)
						{
							//if(data_icon[i] == 0x00010101) data_icon[i] = 0;
							if(data_icon[i] != 0)
							{
								BYTE *pixs = (BYTE*)&data_icon[i];
								//DbgPrint(L"%08X",data_icon[i]);
								if(pixs[3]==0) data_icon[i] = ~data_icon[i];
								//if(pixs[3]==0) data_icon[i] = 0xFF000000;
							}
						}
					}
					
					//BitBlt(hdc,offsetX, offsetY,bitmap.bmWidth,bitmap.bmHeight,iconDC,0,0,SRCCOPY | CAPTUREBLT);
                    AlphaBlend(hdc, offsetX, offsetY,bitmap.bmWidth,bitmap.bmHeight,iconDC,0,0,bitmap.bmWidth,bitmap.bmHeight, m_Blend);

                    DeleteObject(icon);
                    DeleteDC(iconDC);
                }
            }
            DeleteObject(ii.hbmMask);
            DeleteObject(ii.hbmColor);
            DestroyIcon(ci.hCursor);
            //DrawIconEx(mdc,ci.ptScreenPos.x - ptWinPos.x,ci.ptScreenPos.y-ptWinPos.y,ci.hCursor,DI_DEFAULTSIZE,DI_DEFAULTSIZE,0,0,3);
        }
    }
}

void CuttBitmap(DWORD **data,long &Width,long &Height)
{
	DWORD *data_white = *data;

	RECT trans = {0};

	//上
	for(int h = 0;h<Height;h++)
	{
		for(int w = 0;w<Width;w++)
		{
			if( data_white[ h*Width + w] != 0)
			{
				trans.top = h;
				break;
			}
		}
		if(trans.top) break;
	}

	//下
	for(int h = Height - 1;h>0;h--)
	{
		for(int w = 0;w<Width;w++)
		{
			if( data_white[ h*Width + w] != 0)
			{
				trans.bottom = h;
				break;
			}
		}
		if(trans.bottom) break;
	}

	//左
	for(int w = 0;w<Width;w++)
	{
		for(int h = 0;h<Height;h++)
		{
			if( data_white[ h*Width + w] != 0)
			{
				trans.left = w;
				break;
			}
		}
		if(trans.left) break;
	}

	//右
	for(int w = Width-1;w>0;w--)
	{
		for(int h = 0;h<Height;h++)
		{
			if( data_white[ h*Width + w] != 0)
			{
				trans.right = w;
				break;
			}
		}
		if(trans.right) break;
	}

	if(trans.right > trans.left && trans.bottom > trans.top)
	{
		int oldw = Width;
		Width = trans.right - trans.left + 1;
		Height = trans.bottom - trans.top + 1;

		int dwsize = sizeof(DWORD)*Width;
		DWORD *buff = (DWORD*)malloc(dwsize);

		//重新构建图形
		for(int w = 0;w<Height;w++)
		{
			int src = trans.left + (w+trans.top)*oldw;
			int des = w*Width;
			if(des+Width<src)
			{
				memcpy(&data_white[des],&data_white[src],dwsize);
			}
			else
			{
				memcpy(buff,&data_white[src],dwsize);
				memcpy(&data_white[des],buff,dwsize);
			}
		}

		free(buff);
	}
}
void Save2PNG(LPCTSTR lpFileName,DWORD *m_lpBitBmp,int Width,int Height)
{
	BitmapData bmData;
	Bitmap bmp2(Width,Height,PixelFormat32bppARGB);

	Rect rc(0,0,Width,Height);
	bmp2.LockBits(&rc,ImageLockModeWrite,PixelFormat32bppARGB,&bmData);
	memcpy(bmData.Scan0,m_lpBitBmp,Width*Height*4);

	bmp2.UnlockBits(&bmData);

	CLSID pngClsid;
	GetEncoderClsid(L"image/png",&pngClsid);
	bmp2.Save(lpFileName,&pngClsid);
}


/*
unsigned long Swap32(unsigned long n)
{
	return (( n&0xFF000000) >> 24) | (( n&0x000000FF) << 24) | (( n&0x00FF0000) >> 8) | (( n&0x0000FF00) << 8);
}

int Save2PNG(LPCTSTR lpFileName,DWORD *m_lpBitBmp,int Width,int Height)
{
	HANDLE hFile = CreateFile(lpFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if(hFile == INVALID_HANDLE_VALUE ) return FALSE; 
	
	tinydeflate::uint32 len = 0;
	DWORD dwWritten=0; 

	BYTE * buf = (BYTE*)tinydeflate::write_image_to_png_file_in_memory((void*)m_lpBitBmp,Width,Height,4,&len);
	WriteFile(hFile,buf,len,&dwWritten,NULL); 
	free(buf);
	
	DWORD dwWritten=0; 
	DWORD Length;
	DWORD crc32;

	DWORD swap;
	//HEADER
	WriteFile(hFile,&PNG_HEADER,sizeof(PNG_HEADER),&dwWritten,NULL); 

	//IHDR
	BYTE *buf;

	PNG_IHDR ihdr;
	ihdr.Width = Swap32(Width);
	ihdr.Height = Swap32(Height);
	ihdr.BitDepth = 8;
	ihdr.ColorType = 6;
	ihdr.Compression = 0;
	ihdr.Filter = 0;
	ihdr.Interlace = 0;

	Length = sizeof(PNG_IHDR);
	buf = (BYTE*)malloc(sizeof(PNG_BLOCK)-sizeof(DWORD)+Length);

	swap = Swap32(Length);
	memcpy(buf,&swap,sizeof(DWORD));

	memcpy(buf+sizeof(DWORD),PNG_TYPE_IHDR,sizeof(DWORD));
	memcpy(buf+sizeof(DWORD)*2,&ihdr ,Length);

	crc32 = Swap32(CRC32_MEM(buf+sizeof(DWORD),Length+sizeof(DWORD)));
	memcpy(buf+sizeof(DWORD)*2 + Length,&crc32,sizeof(DWORD));

	WriteFile(hFile,buf,sizeof(PNG_BLOCK)-sizeof(DWORD)+Length,&dwWritten,NULL); 
	free(buf);
	//
	int len = Height * Width * sizeof(DWORD);
	buf = (BYTE*)malloc(len+sizeof(DWORD)*3);
	len = tinydeflate::compress_mem_to_mem(buf+sizeof(DWORD)*2,len,m_lpBitBmp,len,tinydeflate::DEFAULT_MAX_PROBES | tinydeflate::WRITE_ZLIB_HEADER);


	swap = Swap32(len);
	memcpy(buf,&swap,sizeof(DWORD));
	memcpy(buf+sizeof(DWORD),&PNG_TYPE_IDAT,sizeof(PNG_TYPE_IDAT));

	crc32 = Swap32(CRC32_MEM(buf+sizeof(DWORD),len+sizeof(DWORD)));
	memcpy(buf+sizeof(DWORD)*2 + len,&crc32,sizeof(DWORD));
	
	WriteFile(hFile,buf,sizeof(PNG_BLOCK)-sizeof(DWORD)+len,&dwWritten,NULL); 
	free(buf);

	//IEND
	WriteFile(hFile,&PNG_FOOTER,sizeof(PNG_FOOTER),&dwWritten,NULL); 
	
	return TRUE;
}
*/
 #pragma warning(disable : 4996)
#include "split.h"

void OnRegisterHotKey(HWND hWnd,const wchar_t *inipath)
{
	UINT mo = 0;
	UINT vk = 0;

	wchar_t tcsHotkey[MAX_PATH];
	GetPrivateProfileString(L"程序设置", _T("快捷键"), L"Ctrl+PrtSc", tcsHotkey, MAX_PATH, inipath);

	StringSplit split(tcsHotkey,L'+');
	for(int i=0;i<split.GetCount();i++)
	{
		wchar_t* str2 = split.GetIndex(i);

		if(wcsicmp(str2,L"Shift")==0) mo |= MOD_SHIFT;
		else if(wcsicmp(str2,L"Ctrl")==0) mo |= MOD_CONTROL;
		else if(wcsicmp(str2,L"Alt")==0) mo |= MOD_ALT;
		else if(wcsicmp(str2,L"Win")==0) mo |= MOD_WIN;

		wchar_t wch = str2[0];
		if (wcslen(str2)==1)
		{
			if(iswalnum(wch)) vk = towupper(wch);
			else vk = LOWORD(VkKeyScan(wch));
		}
		else if (wch=='F'||wch=='f')
		{
			if(iswdigit(str2[1]))  vk = VK_F1 + _ttoi(&str2[1]) - 1;
		}
		else
		{
			if(wcsicmp(str2,L"Left")==0) vk = VK_LEFT;
			else if(wcsicmp(str2,L"Right")==0) vk = VK_RIGHT;
			else if(wcsicmp(str2,L"Up")==0) vk = VK_UP;
			else if(wcsicmp(str2,L"Down")==0) vk = VK_DOWN;

			else if(wcsicmp(str2,L"End")==0) vk = VK_END;
			else if(wcsicmp(str2,L"Home")==0) vk = VK_HOME;

			else if(wcsicmp(str2,L"Tab")==0) vk = VK_TAB;
			else if(wcsicmp(str2,L"Space")==0) vk = VK_SPACE;

			else if(wcsicmp(str2,L"Esc")==0) vk = VK_ESCAPE;
			else if(wcsicmp(str2,L"Delete")==0) vk = VK_DELETE;

			else if(wcsicmp(str2,L"PageUp")==0) vk = VK_PRIOR;
			else if(wcsicmp(str2,L"PageDown")==0) vk = VK_NEXT;

			else if(wcsicmp(str2,L"PrtSc")==0) vk = VK_SNAPSHOT;
		}

	}

	if( IsSystemWin7() ) mo |= 0x4000;

	if(!RegisterHotKey(hWnd, 1000, mo, vk))
	{
		wchar_t Tips[512];
		wsprintf(Tips,L"注册热键 \"%s\" 失败，程序无法通过此热键工作！",tcsHotkey);
		MessageBox(hWnd,Tips,L"提示",MB_OK | MB_ICONWARNING);
	}
}