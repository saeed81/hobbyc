#include<stdio.h>
#include<stdlib.h>
#include<X11/Xlib.h>
#include<X11/keysym.h>
#include<math.h>
#include<unistd.h>
#include<pthread.h>
#include<netcdf.h>
#include<stdarg.h>
#include<time.h>
#include"colormaps_jet.h" 
#include"colormaps_bright.h" 
#include"colormaps_detail.h"
#include"colormaps_ssec.h"
#include"intostr.c"

#define ArrayCount(a) (sizeof(a) / sizeof(a[0]))
#define StringArrayCount(a) (ArrayCount(a) -1)

#define NX (1238)
#define NY (1046)
#define NT  (726)

typedef unsigned int  uint32;
typedef unsigned char uint8;

float  SSH[NX*NY]    = {0.0};
uint8  image32[NX*NY*4] = {0};

enum CLMAP {JET, BRIGHT, RED_BLUE,DETAIL,SSEC,};

struct Colort{
  float red, green, blue, alpha;
  int ncol;
};

struct xdata {
  Display *dsp;
  int screen_num;
  Window win;
  GC gc;                                  
};



uint32 lenstring(char *str){
  if (str == NULL) return 0U;
  uint32 len = 0U;
  while (*str != '\0'){
    len++;
    str++;
  }
  return len;
}

void writetostring(char *buffer, int N, char *str,...){
  if (str == NULL) return;
  va_list vv;
  va_start(vv,str);
  char *tmp = buffer;
  int counter = 0;
  while(*str!='\0'){
    if (counter < N){
      *tmp = *str;
      tmp++;
    }
    counter++;
    str++;
  }
  while((str=va_arg(vv,char *)) != NULL){
    while(*str!='\0'){
      if (counter < N ){
	*tmp = *str;
	tmp++;
      }
      counter++;
      str++;
    }
  }
  va_end(vv);
  return;
}




float minval(float *a, int n ){
  float mv = a[0];
  for (int i=1; i < n;++i){
    if (a[i] <= mv) mv = a[i];
  }
  return mv;
}

float maxval(float *a, int n){
  float mv = a[0];
  for (int i=1; i < n;++i){
    if (a[i] >= mv) mv = a[i];
  }
  return mv;
}

void colortoRGBA(uint32 color, uint8 *red, uint8 *green, uint8 *blue, uint8 *alpha){

  *red = 0x0;
  *green = 0x0;
  *blue = 0x0;
  *alpha = 0x0;

  *alpha = (color >> 24) & (0xff);
  *red   = (color >> 16) & (0xff);
  *green = (color >> 8) & (0xff);
  *blue  = (color) & (0xff);
  return;
}

struct Colort *getcolorfromcolormap(enum CLMAP CLM, int *ncol){
  uint32 nt = 0;
  if (CLM == JET)    nt = ArrayCount(cmap_jet);
  if (CLM == BRIGHT) nt = ArrayCount(cmap_bright);
  if (CLM == DETAIL) nt = ArrayCount(cmap_detail);
  if (CLM == SSEC)   nt = ArrayCount(cmap_ssec);
  if (nt == 0) return NULL;
  struct Colort *scolor = NULL;
  scolor = (struct Colort *)malloc(sizeof(struct Colort)*nt);
  if (scolor == NULL){
    *ncol = 0;
    return NULL;
  }
  int ncount = 0;
  if (CLM == JET){
    for (size_t i= 0; i < (nt-2); i += 3){
      scolor[ncount].red   = (float)(cmap_jet[i]) / 255.0;
      scolor[ncount].green = (float)(cmap_jet[i+1]) / 255.0;
      scolor[ncount].blue  = (float)(cmap_jet[i+2]) / 255.0;
      scolor[ncount].alpha  = 0.0;
      ncount++;
    }
  }
  if (CLM == BRIGHT){
    for (size_t i= 0; i < (nt-2); i += 3){
      scolor[ncount].red   = (float)(cmap_bright[i]) / 255.0;
      scolor[ncount].green = (float)(cmap_bright[i+1]) / 255.0;
      scolor[ncount].blue  = (float)(cmap_bright[i+2]) / 255.0;
      scolor[ncount].alpha  = 0.0;
      ncount++;
    }
  }
  if (CLM == DETAIL){
    for (size_t i= 0; i < (nt-2); i += 3){
      scolor[ncount].red   = (float)(cmap_detail[i]) / 255.0;
      scolor[ncount].green = (float)(cmap_detail[i+1]) / 255.0;
      scolor[ncount].blue  = (float)(cmap_detail[i+2]) / 255.0;
      scolor[ncount].alpha  = 0.0;
      ncount++;
    }
  }
  if (CLM == SSEC){
    for (size_t i= 0; i < (nt-2); i += 3){
      scolor[ncount].red   = (float)(cmap_ssec[i]) / 255.0;
      scolor[ncount].green = (float)(cmap_ssec[i+1]) / 255.0;
      scolor[ncount].blue  = (float)(cmap_ssec[i+2]) / 255.0;
      scolor[ncount].alpha  = 0.0;
      ncount++;
    }
  }
  *ncol = ncount;
  return scolor;
}

void getrgbpoint(float *value, struct Colort *scolor, int nc, uint8 *red, uint8 *green, uint8 *blue){
  if (scolor == NULL) return;
  if (nc == 0 ) return;
  float zred = 0.0, zgreen = 0.0, zblue = 0.0; 
  int idx1;                 
  int idx2;                
  float fractBetween = 0;  
  if(*value <= 0.0){  idx1 = idx2 = 0;}    
  else if(*value >= 1.0){idx1 = idx2 = nc-1; }   
  else{
    *value = *value * (nc);        
    idx1  = floor(*value);          
    idx2  = idx1+1;                
    fractBetween = *value - (float)(idx1); 
    if (idx1 >= (nc-1) ){ idx1 = (nc-1); idx2 = (nc-1);} 
    if (idx2 > (nc-1) ) idx2 = (nc-1);
  }
  zred   = (scolor[idx2].red   - scolor[idx1].red  )*fractBetween + scolor[idx1].red;
  zgreen = (scolor[idx2].green - scolor[idx1].green)*fractBetween + scolor[idx1].green;
  zblue  = (scolor[idx2].blue  - scolor[idx1].blue )*fractBetween + scolor[idx1].blue;
  *red   = (uint8 )(zred * 255.0);
  *green = (uint8 )(zgreen * 255.0);
  *blue  = (uint8 )(zblue * 255.0);
}

float minvall(float *a, long int n ){
  float mv = a[0];
  for (long int i=1; i < n;++i){
    if (a[i] <= mv) mv = a[i];
  }
  return mv;
}

float maxvall(float *a, long int n){
  float mv = a[0];
  for (long int i=1; i < n;++i){
    if (a[i] >= mv) mv = a[i];
  }
  return mv;
}

void fillcolor(uint32 **pcolor, int *ncol){
  uint8 red = 0x0, green = 0x0, blue = 0x0, alpha = 0x0;
  uint32 nt = 0;
  *pcolor = (uint32 *)malloc(nt * sizeof(uint32));
  if (*pcolor == NULL) return;
  int ncount = 0;
  for (size_t i= 0; i < (nt-2); i += 3){
    *pcolor[ncount] = 0;
    red   = cmap_jet[i];
    green = cmap_jet[i+1];
    blue  = cmap_jet[i+2];
    *pcolor[ncount] = (((uint32)red << 16) | ((uint32)green << 8) | ((uint32)blue) | (uint32)alpha<<24);
    ncount++;
  }
  *ncol = ncount;
}

XImage *CreateTrueColorImage(Display *display, Visual *visual, int width, int height)
{
  int i, j;
  char *p=image32;
  
  return XCreateImage(display, visual, 24, ZPixmap, 0, image32, width, height, 32, 0);
}

void processEvent(Display *display, Window window, XImage *ximage, int width, int height)
{
  static char *tir="This is red";
  static char *tig="This is green";
  static char *tib="This is blue";
  XEvent ev;
  XNextEvent(display, &ev);
  switch(ev.type)
    {
    case Expose:
      XPutImage(display, window, DefaultGC(display, 0), ximage, 0, 0, 0, 0, width, height);
      XSetForeground(display, DefaultGC(display, 0), 0x00ff0000); // red
      XDrawString(display, window, DefaultGC(display, 0), 32,     32,     tir, strlen(tir));
      XDrawString(display, window, DefaultGC(display, 0), 32+256, 32,     tir, strlen(tir));
      XDrawString(display, window, DefaultGC(display, 0), 32+256, 32+256, tir, strlen(tir));
      XDrawString(display, window, DefaultGC(display, 0), 32,     32+256, tir, strlen(tir));
      XSetForeground(display, DefaultGC(display, 0), 0x0000ff00); // green
      XDrawString(display, window, DefaultGC(display, 0), 32,     52,     tig, strlen(tig));
      XDrawString(display, window, DefaultGC(display, 0), 32+256, 52,     tig, strlen(tig));
      XDrawString(display, window, DefaultGC(display, 0), 32+256, 52+256, tig, strlen(tig));
      XDrawString(display, window, DefaultGC(display, 0), 32,     52+256, tig, strlen(tig));
      XSetForeground(display, DefaultGC(display, 0), 0x000000ff); // blue
      XDrawString(display, window, DefaultGC(display, 0), 32,     72,     tib, strlen(tib));
      XDrawString(display, window, DefaultGC(display, 0), 32+256, 72,     tib, strlen(tib));
      XDrawString(display, window, DefaultGC(display, 0), 32+256, 72+256, tib, strlen(tib));
      XDrawString(display, window, DefaultGC(display, 0), 32,     72+256, tib, strlen(tib));
      //XFlush(display);
      break;
    case ButtonPress:
      exit(0);
    }
}
void *myThreadFun(void *vargp){
  
  struct xdata *xd = (struct xdata *)vargp; 
  float angle = 0, x1 = 0, y1 = 0;
  float mpi = 4.0*atan(1.0);
  for(;;){
    printf("thread 1 and angle is %f\n",angle);
    angle += 1.0;
    XClearWindow(xd->dsp, xd->win);
    char sangle[64] = {'\0'};
    intostr((int)angle, sangle);
    int length = 0;
    char *tmp = (char *)sangle; 
    while(*tmp != '\0' ){
      length++;
      tmp++;
    }
    x1= 50.0 * cos(-angle* (mpi / 180.0));
    y1= 50.0 * sin(-angle* (mpi / 180.0));
    XDrawLine(xd->dsp, xd->win, xd->gc, 150, 150, 150+2*(int)x1 , 150+2*(int)y1);
    XDrawString(xd->dsp, xd->win, xd->gc, 150+2*(int)x1 + 8 ,  150+2*(int)y1 + 8, sangle, length);
    if (angle > 360.0) angle = 0.0;
    XFlush(xd->dsp); // Tell the graphics server to show us the results now. 
    usleep(100000);  // Wait for 100000 micro seconds                    
  }
    XFlush(xd->dsp);
    return NULL;
}

int main(void){
 
  int ncid  = 0;
  int  issh = 0;
  char *filename  =  "abborre_ssh.nc";
  char *varname   =  "SSH_inst";
  size_t iassh[3] = {0,0,0};
  size_t start[3] = {0,0,0};
  size_t count[3] = {0,0,0};
  nc_open(filename,NC_NOWRITE,&ncid);
  nc_inq_varid(ncid, varname, &issh);
  
  Display *dsp;
  Window win;
  int screen_num;
  GC gc;                                /* handle of newly created GC.  */
  dsp = XOpenDisplay((char *)0);
  screen_num = DefaultScreen(dsp);
  Visual *visual=DefaultVisual(dsp, 0);
  win    = XCreateSimpleWindow (dsp, DefaultRootWindow (dsp),0, 0, NX,NY,0,0xffffffff,0xffffffff);
  XSelectInput(dsp, win, ExposureMask | StructureNotifyMask | KeyPressMask | ButtonPressMask | PointerMotionMask);        // We want to get MapNotify events  

  XGCValues gr_values;
  //XFontStruct *fontinfo = XLoadQueryFont(dsp,"-adobe-times-bold-r-normal--18-180-75-75-p-99-iso8859-9");
  XFontStruct *fontinfo = XLoadQueryFont(dsp,"10x20");
  gr_values.font =   fontinfo->fid;
  gr_values.function =   GXcopy;
  gr_values.plane_mask = AllPlanes;
  gr_values.foreground = BlackPixel(dsp,screen_num);
  gr_values.background = WhitePixel(dsp,screen_num);
  gc=XCreateGC(dsp,win,GCFont | GCFunction | GCPlaneMask | GCForeground | GCBackground,&gr_values);
  XMapWindow(dsp, win);
  // "Map" the window (that is, make it appear on the screen)                                                                                                                     
  for(;;){XEvent e; XNextEvent(dsp,&e); if(e.type == MapNotify) break;} //Wait for the MapNotify event  
  XFlush(dsp);
  //printf("Xpending %d\n",XPending(dsp));
  float minv = 0.0f, maxv = 0.0f;
  uint8 ured = 0x0, ugreen = 0x0, ublue = 0x0, ualpha = 0x0;
  uint32 pcolor = 0x0;
  float value =0.0f;

  int ncolor = 0;
  struct Colort *scolor = getcolorfromcolormap(JET,&ncolor);
  
  if (scolor == NULL) return 1;
  minv = 0.0;
  maxv = 0.0;
  XImage *ximage = NULL;
  uint32 *pimage = NULL;
  char *title = "NEMO_NORDIC_NS01";
  XEvent e;
  int done = 1;
  int k = 0;
  int countertext = 0;
  for(;done;){
    //printf("Xpending %d\n",XPending(dsp));
     if (XPending(dsp) == 0){
       //XClearWindow(dsp,win);
       k= 0;
       pimage = (uint32 *)image32; 
       start[0] = k;
       start[1] = 0;
       start[2] = 0;
       count[0] = 1;
       count[1] = NY;
       count[2] = NX;
       nc_get_vara(ncid, issh,  start,count, SSH);
      for (int j=0; j < NY; ++j){
	for (int i=0; i < NX; ++i){
	  if (SSH[j*NX+i] == 1.e+20f) SSH[j*NX+i] = 0.0;
	}
      }
      minv = minvall(SSH, NX *NY);
      maxv = maxvall(SSH, NX *NY);
      char ckt[64]  = {'\0'};
      char cmin[64] = {'\0'};
      char cmax[64] = {'\0'};
      intostr(k,ckt);
      floatostr(minv,cmin,4);
      floatostr(maxv,cmax,4);
      char infomin[128] = {'\0'};
      char infomax[128] = {'\0'};
      char infokt[128]  = {'\0'};
      writetostring(infokt,StringArrayCount(infokt),"it : ",ckt,NULL);
      writetostring(infomin,StringArrayCount(infomin),"Min : ",cmin,NULL);
      writetostring(infomax,StringArrayCount(infomax),"Max : ",cmax,NULL);
      for (int j=0; j < NY; ++j){
	for (int i=0; i < NX; ++i){
	  value = (SSH[(NY -j -1)*NX+i] - minv) / (maxv - minv);
	  getrgbpoint(&value,scolor, ncolor, &ured, &ugreen, &ublue);
	  pcolor = 0x0;
	  pcolor = (((uint32)ured << 16) | ((uint32)ugreen << 8) | ((uint32)ublue) |  ((uint32)ualpha << 24));  
	  if (SSH[(NY -j -1)*NX+i] == 0.0) pcolor=0xffffffff;
	  *pimage++ = pcolor;
	}
      }
      //printf("kt %d \n",k);
      ximage = XCreateImage(dsp, visual, 24, ZPixmap, 0, image32, NX, NY, 32, 0);
      XPutImage(dsp, win, gc, ximage, 0, 0, 0, 0, NX, NY);
      XFlush(dsp);
      if (countertext == 0){
      XSetForeground(dsp, gc, 0x000000ff); // red
      XDrawString(dsp, win, gc, 125 , 150  , title, lenstring(title));
      XFlush(dsp);
      XSetForeground(dsp, gc, 0x00ffe1e1); // red
      XDrawString(dsp, win, gc, 10 , 50,  infokt, lenstring(infokt));
      XFlush(dsp);
      XSetForeground(dsp, gc, 0x00000000); // red
      XDrawString(dsp, win, gc, 80 , 50,  infomin, lenstring(infomin));
      XFlush(dsp);
      XSetForeground(dsp, gc, 0x00ff0000); // red
      XDrawString(dsp, win, gc, 200 , 50, infomax, lenstring(infomax));
      XFlush(dsp);
      }
      countertext++;
      if (countertext >= 1000 && k == 0) countertext = 0;
      //usleep(1000*200);
     }
     else {
       XNextEvent (dsp,&e);
       if (e.type == KeyPress){
	 int buffer_size = 80;
	 char buffer[80] = {'\0'};
	 KeySym keysym;
	 /* XComposeStatus compose; is not used, though it's in some books */
	 int icount = XLookupString(&e,buffer,(buffer_size)-1, &keysym);
	 
	 if ((buffer[0] == 'q') || ( buffer[0]== 'Q') ){
	   done =0;
	   break;
	 }else{
	     k = 0;
	   for (;;){
	     clock_t t1;
	     t1 = clock();
	     //XClearWindow(dsp,win);
	     pimage = (uint32 *)image32; 
	     start[0] = k;
	     start[1] = 0;
	     start[2] = 0;
	     count[0] = 1;
	     count[1] = NY;
	     count[2] = NX;
	     nc_get_vara(ncid, issh,  start,count, SSH);
	     for (int j=0; j < NY; ++j){
	       for (int i=0; i < NX; ++i){
	  if (SSH[j*NX+i] == 1.e+20f) SSH[j*NX+i] = 0.0;
	}
      }
      minv = minvall(SSH, NX *NY);
      maxv = maxvall(SSH, NX *NY);
      char ckt[64]  = {'\0'};
      char cmin[64] = {'\0'};
      char cmax[64] = {'\0'};
      intostr(k,ckt);
      floatostr(minv,cmin,4);
      floatostr(maxv,cmax,4);
      char infomin[128] = {'\0'};
      char infomax[128] = {'\0'};
      char infokt[128]  = {'\0'};
      writetostring(infokt,StringArrayCount(infokt),"it :",ckt,NULL);
      writetostring(infomin,StringArrayCount(infomin),"Min :",cmin,NULL);
      writetostring(infomax,StringArrayCount(infomax),"Max :",cmax,NULL);
      for (int j=0; j < NY; ++j){
	for (int i=0; i < NX; ++i){
	  value = (SSH[(NY -j -1)*NX+i] - minv) / (maxv - minv);
	  getrgbpoint(&value,scolor, ncolor, &ured, &ugreen, &ublue);
	  pcolor = 0x0;
	  pcolor = (((uint32)ured << 16) | ((uint32)ugreen << 8) | ((uint32)ublue) |  ((uint32)ualpha << 24));  
	  if (SSH[(NY -j -1)*NX+i] == 0.0) pcolor=0xffffffff;
	  *pimage++ = pcolor;
	}
      }
      //printf("kt %d \n",k);
      ximage = XCreateImage(dsp, visual, 24, ZPixmap, 0, image32, NX, NY, 32, 0);
      XPutImage(dsp, win, gc, ximage, 0, 0, 0, 0, NX, NY);
      XFlush(dsp);
      XSetForeground(dsp, gc, 0x000000ff); // red
      XDrawString(dsp, win, gc, 125 , 150  , title, lenstring(title));
      XFlush(dsp);
      XSetForeground(dsp, gc, 0x00ff0101); // red
      XDrawString(dsp, win, gc, 10 , 50,  infokt, lenstring(infokt));
      XFlush(dsp);
      XSetForeground(dsp, gc, 0x00000000); // red
      XDrawString(dsp, win, gc, 100 , 50,  infomin, lenstring(infomin));
      XFlush(dsp);
      XSetForeground(dsp, gc, 0x00ff0000); // red
      XDrawString(dsp, win, gc, 250 , 50 , infomax, lenstring(infomax));
      XFlush(dsp);
      clock_t t2 = clock();
      float time_taken = ((float)(t2 -t1))/CLOCKS_PER_SEC; // in seconds
      char msec[128] = {'\0'};
      floatostr(time_taken*1.0e3,msec,6);
      char secperframe[128]  = {'\0'};
      writetostring(secperframe,StringArrayCount(secperframe),"time per frame : ",msec," ms",NULL);
      XSetForeground(dsp, gc, 0x00ff0000); // red
      XDrawString(dsp, win, gc, 800 , 900 , secperframe, lenstring(secperframe));
      XFlush(dsp);
      int ipend = XPending(dsp);
      //printf("ipend is %d\n",ipend);
      if (ipend >= 1){
	//printf("we break at k %d\n",ipend);
	//XSync(dsp,True);
	for (int ii=0; ii < ipend;++ii){
	  XNextEvent(dsp,&e);
	  if (e.type == ButtonPress ){
	//XSync(dsp,True);
	    printf("it %3d y %3d x %3d SSH(%3d,%3d,%3d) = %f\n",k,e.xbutton.y,e.xbutton.x,k,e.xbutton.y,e.xbutton.x,SSH[(NY -e.xbutton.y -1)*NX+e.xbutton.x]);
	    XFlush(dsp);
	    break;
	  }
	  else{
	    XFlush(dsp);
	  }
	}
      }
      k++;
      //if (e.type == KeyPress ){
      //int buffer_size = 80;
      //char buffer[80] = {'\0'};
      //KeySym keysym;
	/* XComposeStatus compose; is not used, though it's in some books */
	//int icount = XLookupString(&e,buffer,(buffer_size)-1, &keysym);
	//if ((buffer[0] == 'q') || ( buffer[0]== 'Q') ){
      //done = 0;
      //  break;
      //}
      //}
      
      //}
      
      if (k == NT) k = 0;
      usleep(1000*200);
      XFlush(dsp);
      //XSync(dsp,True);
	   }
	 }
       }
     }
  }
  nc_close(ncid);
  XFlush(dsp);
  free(scolor);
  XFreeGC(dsp,gc);
  XDestroyWindow(dsp,win);
  XCloseDisplay(dsp);
  
  return 0;
}
