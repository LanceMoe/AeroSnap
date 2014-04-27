/*
__inline DWORD AlphaMixed(DWORD color1,DWORD color2)
{
	BYTE *bit1 = (BYTE *)&color1;
	BYTE *bit2 = (BYTE *)&color2;

	bit1[0] = bit1[0]*bit1[3]/255 + bit2[0]*(255-bit1[3])/255;
	bit1[1] = bit1[1]*bit1[3]/255 + bit2[1]*(255-bit1[3])/255;
	bit1[2] = bit1[2]*bit1[3]/255 + bit2[2]*(255-bit1[3])/255;
	bit1[3] = 0xff;
	return color1;
}
*/

DWORD MMXAlphaMixed(DWORD src,DWORD dst)
{
    __asm
    {
        //Initialize
        mov eax,0xFF000000 //Alpha mask
        mov ebx,0xFFFFFFFF //255-Alpha mask
        pxor mm7,mm7
        movd mm5,eax
        movd mm6,ebx
        punpcklbw mm5,mm7 //mm5=alpha mask
        punpcklbw mm6,mm7 //mm6=(255-alpha) mask
         
        //Alpha Blend:
        movd mm0,src      //mm0=packed src
        punpcklbw mm0,mm7 //mm0=unpacked src
        movq mm2,mm0      //mm2=unpacked src
        punpckhwd mm0,mm0
        movd mm3,dst      //mm3=packed dst
        punpckhdq mm0,mm0 //mm0=unpacked src alpha bit
        movq mm1,mm6
        punpcklbw mm3,mm7 //mm3=dst
        psubb mm1,mm0     //mm1=255-src alpha bit
        paddusb mm0,mm5   //mm0=current unpacked src alpha bit
        pmullw mm2,mm0    //mm2=src*srcAlpha
        pmullw mm3,mm1    //mm3=dst*(255-srcAlpha)
        paddusw mm3,mm2   //mm3=src*srcAlpha+dst*(255-srcAlpha)
        psrlw mm3,8       //mm3=src*srcAlpha/256+dst*(255-srcAlpha)/256
        packuswb mm3,mm7  //mm3=packed dst
        movd eax,mm3
    }
}