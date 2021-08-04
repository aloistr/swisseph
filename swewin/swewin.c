/*
 *%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *%                                                                       
 *%     Copyright (C) Astrodienst AG
 *%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *
 * SWEWIN.C
 *
 * SWISSEPH test windows program,
 * 
 *  Authors: Dieter Koch and Alois Treindl, Astrodienst AG
 **************************************************************/

/* Copyright (C) 1997 - 2015 Astrodienst AG, Switzerland.  All rights reserved.
  
  License conditions
  ------------------

  This file is part of Swiss Ephemeris.

  Swiss Ephemeris is distributed with NO WARRANTY OF ANY KIND.  No author
  or distributor accepts any responsibility for the consequences of using it,
  or for whether it serves any particular purpose or works at all, unless he
  or she says so in writing.  

  Swiss Ephemeris is made available by its authors under a dual licensing
  system. The software developer, who uses any part of Swiss Ephemeris
  in his or her software, must choose between one of the two license models,
  which are
  a) GNU public license version 2 or later
  b) Swiss Ephemeris Professional License

  The choice must be made before the software developer distributes software
  containing parts of Swiss Ephemeris to others, and before any public
  service using the developed software is activated.

  If the developer choses the GNU GPL software license, he or she must fulfill
  the conditions of that license, which includes the obligation to place his
  or her whole software project under the GNU GPL or a compatible license.
  See http://www.gnu.org/licenses/old-licenses/gpl-2.0.html

  If the developer choses the Swiss Ephemeris Professional license,
  he must follow the instructions as found in http://www.astro.com/swisseph/ 
  and purchase the Swiss Ephemeris Professional Edition from Astrodienst
  and sign the corresponding license contract.

  The License grants you the right to use, copy, modify and redistribute
  Swiss Ephemeris, but only under certain conditions described in the License.
  Among other things, the License requires that the copyright notices and
  this notice be preserved on all copies.

  Authors of the Swiss Ephemeris: Dieter Koch and Alois Treindl

  The authors of Swiss Ephemeris have no control or influence over any of
  the derived works, i.e. over software or services created by other
  programmers which use Swiss Ephemeris functions.

  The names of the authors or of the copyright holder (Astrodienst) must not
  be used for promoting any software, product or service which uses or contains
  the Swiss Ephemeris. This copyright notice is the ONLY place where the
  names of the authors can legally appear, except in cases where they have
  given special permission in writing.

  The trademarks 'Swiss Ephemeris' and 'Swiss Ephemeris inside' may be used
  for promoting such software, products or services.
*/

#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#ifdef __MINGW32__
#include <sys/types.h> 
#include <sys/stat.h>
#else
#include <sys\types.h> 
#include <sys\stat.h>
#endif
#include <direct.h>
#include <time.h>
#include <process.h>

/* windows.h is not identical in watcom and microsoft env */
#ifndef GWW_HINSTANCE           
#define GWW_HINSTANCE GWL_HINSTANCE
#endif

#include "../sweodef.h"
#include "resource.h"
#include "swewin.h"
#include "../swephexp.h"

#define BUFLEN  8000
#define MY_ODEGREE_STRING "°"

static char *etut[]={"UT", "ET"};
static char *lat_n_s[]={"N", "S"};
static char *lon_e_w[]={"E", "W"};
#define NEPHE           3
static char *ephe[]={"Swiss Ephemeris", "JPL Ephemeris DE406", "Moshier Ephemeris"};
#define NPLANSEL        3
static char *plansel[] = {"main planets", "with asteroids", "with hyp. bodies"};
#define NCENTERS	6
static char *ctr[] = {"geocentric", "topocentric", "heliocentric", "barycentric","sidereal Fagan", "sidereal Lahiri"};
#define NHSYS        9
static char *hsysname[] = {"Placidus", "Campanus", "Regiomontanus", "Koch", "Equal", "Vehlow equal", "Horizon", "B=Alcabitus", "I=Sunshine"};

static char classname[32]="SwewinClass";
static struct pd {
    char etut[3];
    char lon_e_w[2];
    char lat_n_s[2];
    char ephe[30];
    char plansel[30];
    char ctr[20];
    char hsysname[30];
	char sast[AS_MAXCH];
    unsigned int mday, mon, hour, min, sec;
    int year;
    unsigned int  lon_deg, lon_min, lon_sec;
    unsigned int  lat_deg, lat_min, lat_sec;
    long  alt;
#if 0
	AS_BOOL helio;
#endif
    } pd, old_pd;

/* forward declarations */
static void init_data(void);
static char *progname ="Swisseph Test Program";

BOOL FAR PASCAL about_proc( HWND, unsigned, UINT, LONG );
BOOL FAR PASCAL inp_setup_proc( HWND, unsigned, UINT, LONG );
long FAR PASCAL WindowProc( HWND, unsigned, UINT, LONG );
static int swisseph(char *buf);
static char *dms(double x, long iflag);
static void do_print(char *target, char *info);
static int letter_to_ipl(int letter);
static int make_ephemeris_path(long iflag, char *argv0);
static int atoulng(char *s, unsigned long *ulng);
static int atoslng(char *s, long *slng);
static int cut_str_any(char *s, char *cutlist, char *cpos[], int nmax);

#define J2000           2451545.0  /* 2000 January 1.5 */
#define square_sum(x)   (x[0]*x[0]+x[1]*x[1]+x[2]*x[2])
#define SEFLG_EPHMASK   (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH)

#define BIT_ROUND_SEC   1
#define BIT_ROUND_MIN   2
#define BIT_ZODIAC      4
#define PLSEL_D  "0123456789mtABC"
#define PLSEL_P  "0123456789mtABCDEFGHI"
#define PLSEL_H  "JKLMNOPQRSTUVWX"
#define PLSEL_A  "0123456789mtABCDEFGHIJKLMNOPQRSTUVWX"

extern char FAR *pgmptr;
static char *zod_nam[] = {"ar", "ta", "ge", "cn", "le", "vi", 
                          "li", "sc", "sa", "cp", "aq", "pi"};

int PASCAL WinMain( HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline,
                    int cmdshow )
/*******************************/
{
    MSG         msg;
    HWND        hwnd;
    WNDCLASS    wndclass;
    char *argv0 = "";
    cmdline = cmdline;          /* shut up compiler warning */
    prev_inst = prev_inst;

#ifdef __WINDOWS_386__
    sprintf( classname,"SwewinClass%d", this_inst );
#else
    if( !prev_inst ) {
#endif
        wndclass.style          = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc    = (LPVOID) WindowProc;
        wndclass.cbClsExtra     = 0;
        wndclass.cbWndExtra     = 6 * sizeof( DWORD );
        wndclass.hInstance      = this_inst;
        wndclass.hIcon          = LoadIcon(this_inst, IDI_APPLICATION);
        wndclass.hCursor        = LoadCursor( NULL, IDC_ARROW );
        wndclass.hbrBackground  = (HBRUSH) COLOR_BACKGROUND;
        wndclass.lpszMenuName   = "sweph_menu";
        wndclass.lpszClassName  = classname;
        RegisterClass( &wndclass );
#ifndef __WINDOWS_386__
    }
#endif
    hwnd = CreateWindow( classname, progname,
                         WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                         340, 200,
                         NULL, NULL, this_inst, NULL );
    ShowWindow( hwnd, cmdshow );
    UpdateWindow( hwnd );
    init_data();
    if (make_ephemeris_path(SEFLG_SWIEPH | SEFLG_SPEED, argv0) == ERR) {
      MessageBox(hwnd, "error in make_ephemeris_path()", progname, MB_OK);
      exit(1);
    }
#if 0
    {
    char serr[AS_MAXCH];
    if (find_ephemeris_path(0, argv0, serr) == ERR) {
      MessageBox(hwnd, serr, progname, MB_OK);
      exit(1);
    }
    }
#endif
    while( GetMessage( &msg, NULL, (UINT) NULL, (UINT) NULL ) ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
    /* close open files and free allocated space */
    swe_close();
    return( msg.wParam );
} /* WinMain */

/*
 * WindowProc - handle messages for the main application window
 */
LONG FAR PASCAL WindowProc( HWND window_handle, unsigned msg,
                                     UINT wparam, LONG lparam )
/*************************************************************/
{
    DLGPROC             proc;
    HANDLE              inst_handle;
    WORD                cmd;
    /*
     * now process the message
     */
    switch( msg ) {
    case WM_CREATE:
        inst_handle = GET_HINST( window_handle );
        break;
    case WM_CLOSE:
        return( DefWindowProc( window_handle, msg, wparam, lparam ) );
        break;
    case WM_COMMAND:
        cmd = LOWORD( wparam );
        switch( cmd ) {
        case MENU_ABOUT:
            inst_handle = GET_HINST( window_handle );
            proc = MakeProcInstance( about_proc, inst_handle );
            DialogBox( inst_handle,"AboutBx", window_handle, proc );
            FreeProcInstance( proc );
            break;
        case MENU_EXIT:
            SendMessage( window_handle, WM_CLOSE, 0, 0L );
            break;
        case MENU_CALC:
            inst_handle = GET_HINST( window_handle );
            proc = MakeProcInstance( inp_setup_proc, inst_handle );
            DialogBox( inst_handle, "DATABX", window_handle, proc );
            FreeProcInstance( proc );
            SendMessage( window_handle, WM_CLOSE, 0, 0L );
            break;
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

    default:
        return( DefWindowProc( window_handle, msg, wparam, lparam ) );
    }
    return( 0L );

} /* WindowProc */

/*
 * about_proc -  processes messages for the about dialogue.
 */
BOOL FAR PASCAL about_proc( HWND window_handle, unsigned msg,
                                UINT wparam, LONG lparam )
/********************************************************/
{
    char s[2 * AS_MAXCH];
    char s1[AS_MAXCH];
    char s2[AS_MAXCH];
    lparam = lparam;                    /* turn off warning */
    window_handle = window_handle;
    switch( msg ) {
    case WM_INITDIALOG:
	swe_version(s1);
	swe_get_library_path(s2);
	sprintf(s, "Version: %s\nPath %s", s1, s2);
        SetDlgItemText( window_handle, ABOUT_L3, s);
        return( TRUE );
    case WM_CLOSE :
        EndDialog( window_handle, TRUE );
        return( TRUE );
    case WM_COMMAND:
        if( LOWORD( wparam ) == IDOK ) {
            EndDialog( window_handle, TRUE );
            return( TRUE );
        }
        break;
    }
    return( FALSE );
} /* About */

/*
 * Input
 */
extern BOOL FAR PASCAL inp_setup_proc( HWND hdlg, UINT message,
                                                 UINT wparam, LONG lparam )
/*************************************************************************/
{
    WORD        cmd;
    int         i, j;
    long	slng;
    unsigned long	ulng;
    char s[AS_MAXCH];
    char *buf;
    lparam = lparam;    /* shut compiler up*/
    switch( message ) {
    case WM_INITDIALOG :
        old_pd = pd;
        for (i = j = 0; i < 2; i++) {
          if (strcmp(lat_n_s[i], pd.lat_n_s) == 0) j = i;
          SendDlgItemMessage( hdlg, COMBO_N_S, CB_ADDSTRING, (UINT) NULL, (DWORD)(LPSTR) lat_n_s[i]);
        }
        SendDlgItemMessage( hdlg, COMBO_N_S, CB_SETCURSEL, j, (long) NULL);
        for (i = j = 0; i < 2; i++) {
          if (strcmp(lon_e_w[i], pd.lon_e_w) == 0) j = i;
          SendDlgItemMessage( hdlg, COMBO_E_W, CB_ADDSTRING, (UINT) NULL, (DWORD)(LPSTR) lon_e_w[i]);
        }
        SendDlgItemMessage( hdlg, COMBO_E_W, CB_SETCURSEL, j, (long) NULL);
        for (i = j = 0; i < 2; i++) {
          if (strcmp(etut[i], pd.etut) == 0) j = i;
          SendDlgItemMessage( hdlg, COMBO_ET_UT, CB_ADDSTRING, (UINT) NULL, (DWORD)(LPSTR) etut[i]);
        }
        SendDlgItemMessage( hdlg, COMBO_ET_UT, CB_SETCURSEL, j, (long) NULL);
        for (i = j = 0; i < NEPHE; i++) {
          if (strcmp(ephe[i], pd.ephe) == 0) j = i;
          SendDlgItemMessage( hdlg, COMBO_EPHE, CB_ADDSTRING, (UINT) NULL, (DWORD)(LPSTR) ephe[i]);
        }
        SendDlgItemMessage( hdlg, COMBO_EPHE, CB_SETCURSEL, j, (long) NULL);
        for (i = j = 0; i < NPLANSEL; i++) {
          if (strcmp(plansel[i], pd.plansel) == 0) j = i;
          SendDlgItemMessage( hdlg, COMBO_PLANSEL, CB_ADDSTRING, (UINT) NULL, (DWORD)(LPSTR) plansel[i]);
        }
        SendDlgItemMessage( hdlg, COMBO_PLANSEL, CB_SETCURSEL, j, (long) NULL);
        for (i = j = 0; i < NCENTERS; i++) {
          if (strcmp(ctr[i], pd.ctr) == 0) j = i;
          SendDlgItemMessage( hdlg, COMBO_CENTER, CB_ADDSTRING, (UINT) NULL, (DWORD)(LPSTR) ctr[i]);
        }
        SendDlgItemMessage( hdlg, COMBO_CENTER, CB_SETCURSEL, j, (long) NULL);
        for (i = j = 0; i < NHSYS; i++) {
          if (strcmp(hsysname[i], pd.hsysname) == 0) j = i;
          SendDlgItemMessage( hdlg, COMBO_HSYS, CB_ADDSTRING, (UINT) NULL, (DWORD)(LPSTR) hsysname[i]);
        }
        SendDlgItemMessage( hdlg, COMBO_HSYS, CB_SETCURSEL, j, (long) NULL);
        /* set date */
        SetDlgItemText( hdlg, EDIT_DAY, itoa(pd.mday, s, 10));
        SetDlgItemText( hdlg, EDIT_MONTH, itoa(pd.mon, s, 10));
        SetDlgItemText( hdlg, EDIT_YEAR, itoa(pd.year, s, 10));
        SetDlgItemText( hdlg, EDIT_HOUR, itoa(pd.hour, s, 10));
        SetDlgItemText( hdlg, EDIT_MINUTE, itoa(pd.min, s, 10));
        SetDlgItemText( hdlg, EDIT_SECOND, itoa(pd.sec, s, 10));
        SetDlgItemText( hdlg, EDIT_LONG, itoa(pd.lon_deg, s, 10));
        SetDlgItemText( hdlg, EDIT_LONGM, itoa(pd.lon_min, s, 10));
        SetDlgItemText( hdlg, EDIT_LONGS, itoa(pd.lon_sec, s, 10));
        SetDlgItemText( hdlg, EDIT_LAT, itoa(pd.lat_deg, s, 10));
        SetDlgItemText( hdlg, EDIT_LATM, itoa(pd.lat_min, s, 10));
        SetDlgItemText( hdlg, EDIT_LATS, itoa(pd.lat_sec, s, 10));
        SetDlgItemText( hdlg, EDIT_ALT, ltoa(pd.alt, s, 10));
        SetDlgItemText( hdlg, EDIT_ASTNO, pd.sast);
        return( TRUE );
    case WM_CLOSE :
        EndDialog( hdlg, TRUE );
        return( TRUE );
    case WM_COMMAND :
        cmd = LOWORD( wparam );
        switch( cmd ) {
        case PB_EXIT :
            pd = old_pd;
            EndDialog( hdlg, 0 );
            return( TRUE );
        case PB_DOIT :     
            buf = (char *)calloc(BUFLEN, sizeof(char));
            swisseph(buf);
            SetDlgItemText(hdlg, EDIT_OUTPUT2, buf);
            free(buf);
            return( TRUE );
        case COMBO_ET_UT:
            i = (int) SendDlgItemMessage(hdlg, COMBO_ET_UT, CB_GETCURSEL, (UINT) NULL, (long) NULL);
            strcpy(pd.etut, etut[i]);
            return TRUE;
        case COMBO_N_S:
            i = (int) SendDlgItemMessage( hdlg, COMBO_N_S, CB_GETCURSEL, (UINT) NULL, (long) NULL);
            strcpy(pd.lat_n_s, lat_n_s[i]);
            return TRUE;
        case COMBO_E_W:
            i = (int) SendDlgItemMessage( hdlg, COMBO_E_W, CB_GETCURSEL, (UINT) NULL, (long) NULL);
            strcpy(pd.lon_e_w, lon_e_w[i]);
            return TRUE;
        case COMBO_EPHE:
            i = (int) SendDlgItemMessage( hdlg, COMBO_EPHE, CB_GETCURSEL, (UINT) NULL, (long) NULL);
            strcpy(pd.ephe, ephe[i]);
            return TRUE;
        case COMBO_PLANSEL:
            i = (int) SendDlgItemMessage( hdlg, COMBO_PLANSEL, CB_GETCURSEL, (UINT) NULL, (long) NULL);
            strcpy(pd.plansel, plansel[i]);
            return TRUE;
        case COMBO_CENTER:
            i = (int) SendDlgItemMessage( hdlg, COMBO_CENTER, CB_GETCURSEL, (UINT) NULL, (long) NULL);
            strcpy(pd.ctr, ctr[i]);
            return TRUE;
        case COMBO_HSYS:
            i = (int) SendDlgItemMessage( hdlg, COMBO_HSYS, CB_GETCURSEL, (UINT) NULL, (long) NULL);
            strcpy(pd.hsysname, hsysname[i]);
            return TRUE;
        case EDIT_DAY:
            GetDlgItemText( hdlg, cmd, s, 3);
            if (*s != '\0') {
              if (atoulng(s, &ulng) == OK && ulng > 0 && ulng <= 31)
                pd.mday = (unsigned int) ulng;
              else
                SetDlgItemText( hdlg, cmd, "");
            }
            return TRUE;
        case EDIT_MONTH:
            GetDlgItemText( hdlg, cmd, s, 3);
            if (*s != '\0') {
              if (atoulng(s, &ulng) == OK && ulng > 0 && ulng <= 12)
                pd.mon = (unsigned int) ulng;
              else
                SetDlgItemText( hdlg, cmd, "");
            }
            return TRUE;
        case EDIT_YEAR:
            GetDlgItemText( hdlg, cmd, s, 6);
            if (*s != '\0') {
              if (atoslng(s, &slng) == OK )
                pd.year = (int) slng;
              else
                SetDlgItemText( hdlg, cmd, "");
            }
            pd.year = atoi(s);
            return TRUE;
        case EDIT_HOUR:
            GetDlgItemText( hdlg, cmd, s, 3);
            if (*s != '\0') {
              if (atoulng(s, &ulng) == OK && ulng < 24)
                pd.hour = (unsigned int) ulng;
              else
                SetDlgItemText( hdlg, cmd, "");
            }
            return TRUE;
        case EDIT_MINUTE:
            GetDlgItemText( hdlg, cmd, s, 3);
            if (*s != '\0') {
              if (atoulng(s, &ulng) == OK && ulng < 60)
                pd.min = (unsigned int) ulng;
              else
                SetDlgItemText( hdlg, cmd, "");
            }
            return TRUE;
        case EDIT_SECOND:
            GetDlgItemText( hdlg, cmd, s, 3);
            if (*s != '\0') {
              if (atoulng(s, &ulng) == OK && ulng < 60)
                pd.sec = (unsigned int) ulng;
              else
                SetDlgItemText( hdlg, cmd, "");
            }
            return TRUE;
        case EDIT_LONG:
            GetDlgItemText( hdlg, cmd, s, 4);
            if (*s != '\0') {
              if (atoulng(s, &ulng) == OK && ulng <= 180)
                pd.lon_deg = (unsigned int) ulng;
              else
                SetDlgItemText( hdlg, cmd, "");
            }
            return TRUE;
        case EDIT_LONGM:
            GetDlgItemText( hdlg, cmd, s, 3);
            if (*s != '\0') {
              if (atoulng(s, &ulng) == OK && ulng < 60)
                pd.lon_min = (unsigned int) ulng;
              else
                SetDlgItemText( hdlg, cmd, "");
            }
            return TRUE;
        case EDIT_LONGS:
            GetDlgItemText( hdlg, cmd, s, 3);
            if (*s != '\0') {
              if (atoulng(s, &ulng) == OK && ulng < 60)
                pd.lon_sec = (unsigned int) ulng;
              else
                SetDlgItemText( hdlg, cmd, "");
            }
            return TRUE;
        case EDIT_LAT:
            GetDlgItemText( hdlg, cmd, s, 3);
            if (*s != '\0') {
              if (atoulng(s, &ulng) == OK && ulng <= 90)
                pd.lat_deg = (unsigned int) ulng;
              else
                SetDlgItemText( hdlg, cmd, "");
            }
            return TRUE;
        case EDIT_LATM:
            GetDlgItemText( hdlg, cmd, s, 3);
            if (*s != '\0') {
              if (atoulng(s, &ulng) == OK && ulng < 60)
                pd.lat_min = (unsigned int) ulng;
              else
                SetDlgItemText( hdlg, cmd, "");
            }
            return TRUE;
        case EDIT_LATS:
            GetDlgItemText( hdlg, cmd, s, 3);
            if (*s != '\0') {
              if (atoulng(s, &ulng) == OK && ulng < 60)
                pd.lat_sec = (unsigned int) ulng;
              else
                SetDlgItemText( hdlg, cmd, "");
            }
            return TRUE;
        case EDIT_ALT:
            GetDlgItemText( hdlg, cmd, s, 10);
            if (*s != '\0') {
              if (atoslng(s, &slng) == OK)
                pd.alt = slng;
              else
                SetDlgItemText( hdlg, cmd, "");
            }
            pd.alt = atol(s);
            return TRUE;
		case EDIT_ASTNO:
		    GetDlgItemText( hdlg, cmd, s, 50);
			strcpy(pd.sast, s);
	        /*SetDlgItemText( hdlg, cmd, pd.sast);*/
		    return TRUE;
        }
        return( FALSE );
    }
    return( FALSE );
}

static void init_data(void)
{
  time_t time_of_day;
  struct tm tmbuf;
  time_of_day = time(NULL);
  tmbuf = *gmtime(&time_of_day);
  pd.mday = tmbuf.tm_mday;
  pd.mon = tmbuf.tm_mon + 1;
  pd.year = tmbuf.tm_year + 1900;
  pd.hour = tmbuf.tm_hour;
  pd.min = tmbuf.tm_min;
  pd.sec = tmbuf.tm_sec;
  /* coordinates of Zurich */
  pd.lon_deg = 8;
  pd.lon_min = 33;
  pd.lon_sec = 0;
  pd.lat_deg = 47;
  pd.lat_min = 23;
  pd.lat_sec = 0;
  pd.alt = 0;
  strcpy(pd.etut, etut[0]);
  strcpy(pd.lat_n_s, lat_n_s[0]);
  strcpy(pd.lon_e_w, lon_e_w[0]);
  strcpy(pd.ephe, ephe[0]);
  strcpy(pd.plansel, plansel[0]);
  strcpy(pd.ctr, ctr[0]);
  strcpy(pd.hsysname, hsysname[0]);
  strcpy(pd.sast, "433, 3045, 7066");
}

static int swisseph(char *buf)
{
  char serr[AS_MAXCH*2], serr_save[AS_MAXCH], serr_warn[AS_MAXCH];
  char s[AS_MAXCH]; 
  char s1[AS_MAXCH], s2[AS_MAXCH];
  char star[AS_MAXCH];
  char *sp, *sp2;
  char se_pname[AS_MAXCH];
  char *spnam, *spnam2 = "";
  char *fmt = "PZBRS";
  char *plsel, *psp;
  char *gap = " ";
  double jut = 0.0, y_frac;
  int i, j;
  double hpos;
  int jday, jmon, jyear, jhour, jmin, jsec;
  int ipl, ipldiff = SE_SUN;
  double x[6], xequ[6], xcart[6], xcartq[6];
  double cusp[12+1];    /* cusp[0] + 12 houses */
  double ascmc[10];		/* asc, mc, vertex ...*/
  double ar, sinp;
  double a, sidt, armc, lon, lat;
  double eps_true, eps_mean, nutl, nuto;
  char ephepath[AS_MAXCH];
  char fname[AS_MAXCH];
  char splan[100], sast[AS_MAXCH];
  int nast, iast;
  long astno[100];
  long iflag = 0, iflag2;              /* external flag: helio, geo... */
  long iflgret;
  long whicheph = SEFLG_SWIEPH;
  AS_BOOL universal_time = FALSE;
  AS_BOOL calc_house_pos = FALSE;
  short gregflag;
  AS_BOOL diff_mode = FALSE;
  int round_flag = 0;
  double tjd_ut = 2415020.5;
  double tjd_et, t2;
  double delt;
  char bc[20];
  char *jul;
  int hsys = (int) *pd.hsysname;
  *serr = *serr_save = *serr_warn = '\0';
  strcpy(ephepath, SE_EPHE_PATH);
  if (strcmp(pd.ephe, ephe[1]) == 0) {
    whicheph = SEFLG_JPLEPH;
    strcpy(fname, SE_FNAME_DE406);
  } else if (strcmp(pd.ephe, ephe[0]) == 0) 
    whicheph = SEFLG_SWIEPH;
  else
    whicheph = SEFLG_MOSEPH;
  if (strcmp(pd.etut, "UT") == 0)
    universal_time = TRUE;
  if (strcmp(pd.plansel, plansel[0]) == 0) {
    plsel = PLSEL_D;
  } else if (strcmp(pd.plansel, plansel[1]) == 0) {
    plsel = PLSEL_P;
  } else if (strcmp(pd.plansel, plansel[2]) == 0) {
    plsel = PLSEL_A;
  }
  if (strcmp(pd.ctr, ctr[0]) == 0)
    calc_house_pos = TRUE;
  else if (strcmp(pd.ctr, ctr[1]) == 0) {
    iflag |= SEFLG_TOPOCTR;
    calc_house_pos = TRUE;
  } else if (strcmp(pd.ctr, ctr[2]) == 0) {
    iflag |= SEFLG_HELCTR;
  } else if (strcmp(pd.ctr, ctr[3]) == 0) {
    iflag |= SEFLG_BARYCTR;
  } else if (strcmp(pd.ctr, ctr[4]) == 0) {
    iflag |= SEFLG_SIDEREAL;
	swe_set_sid_mode(SE_SIDM_FAGAN_BRADLEY, 0, 0);
  } else if (strcmp(pd.ctr, ctr[5]) == 0) {
    iflag |= SEFLG_SIDEREAL;
	swe_set_sid_mode(SE_SIDM_LAHIRI, 0, 0);
#if 0
  } else {
    iflag &= ~(SEFLG_HELCTR | SEFLG_BARYCTR | SEFLG_TOPOCTR);
#endif
  }
  lon = pd.lon_deg + pd.lon_min / 60.0 + pd.lon_sec / 3600.0;
  if (*pd.lon_e_w == 'W')
    lon = -lon;
  lat = pd.lat_deg + pd.lat_min / 60.0 + pd.lat_sec / 3600.0;
  if (*pd.lat_n_s == 'S')
    lat = -lat;
  sprintf(s, "Planet Positions from %s \n\n", pd.ephe);
  do_print(buf, s);                               
  if (whicheph & SEFLG_JPLEPH)
    swe_set_jpl_file(fname);
  iflag = (iflag & ~SEFLG_EPHMASK) | whicheph;
  iflag |= SEFLG_SPEED;
#if 0
  if (pd.helio) iflag |= SEFLG_HELCTR;
#endif
  if ((long) pd.year * 10000L + (long) pd.mon * 100L + (long) pd.mday < 15821015L) 
    gregflag = FALSE;
  else
    gregflag = TRUE;
  jday = pd.mday;
  jmon = pd.mon;
  jyear = pd.year;
  jhour = pd.hour;
  jmin = pd.min;
  jsec = pd.sec;
  jut = jhour + jmin / 60.0 + jsec / 3600.0;
  tjd_ut = swe_julday(jyear,jmon,jday,jut,gregflag);
  swe_revjul(tjd_ut, gregflag, &jyear, &jmon, &jday, &jut);
  jut += 0.5 / 3600;
  jhour = (int) jut;
  jmin = (int) fmod(jut * 60, 60);
  jsec = (int) fmod(jut * 3600, 60);
  *bc = '\0';
  if (pd.year <= 0)
    sprintf(bc, "(%d B.C.)", 1 - jyear);
  if (jyear * 10000L + jmon * 100L + jday <= 15821004)
    jul = "jul.";
  else
    jul = "";
  sprintf(s, "%d.%d.%d %s %s    %02d:%02d:%02d %s\n",
    jday, jmon, jyear, bc, jul,
    jhour, jmin, jsec, pd.etut);
  do_print(buf, s);
  jut = jhour + jmin / 60.0 + jsec / 3600.0;
  if (universal_time) {
    delt = swe_deltat(tjd_ut);
    sprintf(s, " delta t: %f sec", delt * 86400.0);
    do_print(buf, s);
    tjd_et = tjd_ut + delt;
  } else
    tjd_et = tjd_ut;
  sprintf(s, " jd (ET) = %f\n", tjd_et);
  do_print(buf, s);
  iflgret = swe_calc(tjd_et, SE_ECL_NUT, iflag, x, serr);
  eps_true = x[0];
  eps_mean = x[1];
  strcpy(s1, dms(eps_true, round_flag));
  strcpy(s2, dms(eps_mean, round_flag));
  sprintf(s, "\n%-15s %s%s%s    (true, mean)", "Ecl. obl.", s1, gap, s2);
  do_print(buf, s);
  nutl = x[2];
  nuto = x[3];
  strcpy(s1, dms(nutl, round_flag));
  strcpy(s2, dms(nuto, round_flag));
  sprintf(s, "\n%-15s %s%s%s    (dpsi, deps)", "Nutation", s1, gap, s2);
  do_print(buf, s);
  do_print(buf, "\n\n");
  do_print(buf, "               ecl. long.       ecl. lat.   ");
  do_print(buf, "    dist.          speed");
  if (calc_house_pos)
    do_print(buf, "          house");
  do_print(buf, "\n");
  if (iflag & SEFLG_TOPOCTR)
    swe_set_topo(lon, lat, pd.alt);
  sidt = swe_sidtime(tjd_ut) + lon / 15;
  if (sidt >= 24)
    sidt -= 24;
  if (sidt < 0)
    sidt += 24;
  armc = sidt * 15;
  /* additional asteroids */
  strcpy(splan, plsel);
  if (strcmp(plsel,PLSEL_P) == 0) {
    char *cpos[40];
    strcpy(sast, pd.sast);
    j = cut_str_any(sast, ",;. \t", cpos, 40);
    for (i = 0, nast = 0; i < j; i++) {
      if ((astno[nast] = atol(cpos[i])) > 0) {
	nast++;
	strcat(splan, "+");
      }
    }
  }
  for (psp = splan, iast = 0; *psp != '\0'; psp++) {
    if (*psp == '+') {
      ipl = SE_AST_OFFSET + (int) astno[iast];
	  iast++;
    } else
      ipl = letter_to_ipl(*psp);
    if (iflag & SEFLG_HELCTR) {
      if (ipl == SE_SUN
        || ipl == SE_MEAN_NODE || ipl == SE_TRUE_NODE
        || ipl == SE_MEAN_APOG || ipl == SE_OSCU_APOG)
      continue;
    } else if (iflag & SEFLG_BARYCTR) {
      if (ipl == SE_MEAN_NODE || ipl == SE_TRUE_NODE
        || ipl == SE_MEAN_APOG || ipl == SE_OSCU_APOG)
      continue;
    } else          /* geocentric */
      if (ipl == SE_EARTH)
        continue;
    /* ecliptic position */
    if (ipl == SE_FIXSTAR) {
      iflgret = swe_fixstar(star, tjd_et, iflag, x, serr);
      strcpy(se_pname, star);
    } else {
      iflgret = swe_calc(tjd_et, ipl, iflag, x, serr);
      swe_get_planet_name(ipl, se_pname);
	  if (ipl > SE_AST_OFFSET) {
   	    sprintf(s, "#%d", (int) astno[iast-1]);
		strcat(se_pname, "            ");
		strcpy(se_pname + 11 - strlen(s), s);
	  }
    }
    if (iflgret >= 0) {
      if (calc_house_pos) {
        hpos = swe_house_pos(armc, lat, eps_true, hsys, x, serr);
        if (hpos == 0)
          iflgret = ERR;
      }
    }
    if (iflgret < 0) {
      if (*serr != '\0' && strcmp(serr, serr_save) != 0) {
        strcpy (serr_save, serr);
        do_print(buf, "error: ");
        do_print(buf, serr);
        do_print(buf, "\n");
      }
    } else if (*serr != '\0' && *serr_warn == '\0')
      strcpy(serr_warn, serr);
    /* equator position */
    if (strpbrk(fmt, "aADdQ") != NULL) {
      iflag2 = iflag | SEFLG_EQUATORIAL;
      if (ipl == SE_FIXSTAR)
        iflgret = swe_fixstar(star, tjd_et, iflag2, xequ, serr);
      else
        iflgret = swe_calc(tjd_et, ipl, iflag2, xequ, serr);
    }
    /* ecliptic cartesian position */
    if (strpbrk(fmt, "XU") != NULL) {
      iflag2 = iflag | SEFLG_XYZ;
      if (ipl == SE_FIXSTAR)
        iflgret = swe_fixstar(star, tjd_et, iflag2, xcart, serr);
      else
        iflgret = swe_calc(tjd_et, ipl, iflag2, xcart, serr);
    }
    /* equator cartesian position */
    if (strpbrk(fmt, "xu") != NULL) {
      iflag2 = iflag | SEFLG_XYZ | SEFLG_EQUATORIAL;
      if (ipl == SE_FIXSTAR)
        iflgret = swe_fixstar(star, tjd_et, iflag2, xcartq, serr);
      else
        iflgret = swe_calc(tjd_et, ipl, iflag2, xcartq, serr);
    }
    spnam = se_pname;
    /*
     * The string fmt contains a sequence of format specifiers;
     * each character in fmt creates a column, the columns are
     * sparated by the gap string.
     */
    for (sp = fmt; *sp != '\0'; sp++) {
      if (sp != fmt) 
        do_print(buf, gap);
      switch(*sp) {
      case 'y':
          sprintf(s, "%d", jyear);
          do_print(buf, s);
          break;
      case 'Y':
          jut = 0;
          t2 = swe_julday(jyear,1,1,jut,gregflag);
          y_frac = (tjd_ut - t2) / 365.0;
          sprintf(s, "%.2lf", jyear + y_frac);
          do_print(buf, s);
          break;
      case 'p':
          if (diff_mode)
            sprintf(s, "%d-%d", ipl, ipldiff);
          else
            sprintf(s, "%d", ipl);
          do_print(buf, s);
          break;
      case 'P':
          if (diff_mode)
            sprintf(s, "%.3s-%.3s", spnam, spnam2);
          else
            sprintf(s, "%-11s", spnam);
          do_print(buf, s);
          break;
      case 'J':
      case 'j':
          sprintf(s, "%.2f", tjd_ut);
          do_print(buf, s);
          break;
      case 'T':
          sprintf(s, "%02d.%02d.%d", jday, jmon, jyear);
          do_print(buf, s);
          break;
      case 't':
          sprintf(s, "%02d%02d%02d", jyear % 100, jmon, jday);
          do_print(buf, s);
          break;
      case 'L':
          do_print(buf, dms(x[0], round_flag));
          break;
      case 'l':
          sprintf(s, "%# 11.7f", x[0]);
          do_print(buf, s);
          break;
      case 'Z':
          do_print(buf, dms(x[0], round_flag|BIT_ZODIAC));
          break;
      case 'S':
      case 's':
          if (*(sp+1) == 'S' || *(sp+1) == 's' || strpbrk(fmt, "XUxu") != NULL) {
            for (sp2 = fmt; *sp2 != '\0'; sp2++) {
              if (sp2 != fmt) 
                do_print(buf, gap);
              switch(*sp2) {
                case 'L':       /* speed! */
                case 'Z':       /* speed! */
                  do_print(buf, dms(x[3], round_flag));
                  break;
                case 'l':       /* speed! */
                  sprintf(s, "%11.7f", x[3]);
                  do_print(buf, s);
                  break;
                case 'B':       /* speed! */
                  do_print(buf, dms(x[4], round_flag));
                  break;
                case 'b':       /* speed! */
                  sprintf(s, "%11.7f", x[4]);
                  do_print(buf, s);
                  break;
                case 'A':       /* speed! */
                  do_print(buf, dms(xequ[3]/15, round_flag|SEFLG_EQUATORIAL));
                  break;
                case 'a':       /* speed! */
                  sprintf(s, "%11.7f", xequ[3]);
                  do_print(buf, s);
                  break;
                case 'D':       /* speed! */
                  do_print(buf, dms(xequ[4], round_flag));
                  break;
                case 'd':       /* speed! */
                  sprintf(s, "%11.7f", xequ[4]);
                  do_print(buf, s);
                  break;
                case 'R':       /* speed! */
                case 'r':       /* speed! */
                  sprintf(s, "%# 14.9f", x[5]);
                  do_print(buf, s);
                  break;
                case 'U':       /* speed! */
                case 'X':       /* speed! */
                  if (*sp =='U') 
                    ar = sqrt(square_sum(xcart));
                  else 
                    ar = 1;
                  sprintf(s, "%# 14.9f%s", xcart[3]/ar, gap);
                  do_print(buf, s);
                  sprintf(s, "%# 14.9f%s", xcart[4]/ar, gap);
                  do_print(buf, s);
                  sprintf(s, "%# 14.9f", xcart[5]/ar);
                  do_print(buf, s);
                  break;
                case 'u':       /* speed! */
                case 'x':       /* speed! */
                  if (*sp =='u') 
                    ar = sqrt(square_sum(xcartq));
                  else 
                    ar = 1;
                  sprintf(s, "%# 14.9f%s", xcartq[3]/ar, gap);
                  do_print(buf, s);
                  sprintf(s, "%# 14.9f%s", xcartq[4]/ar, gap);
                  do_print(buf, s);
                  sprintf(s, "%# 14.9f", xcartq[5]/ar);
                  do_print(buf, s);
                  break;
                default:
                  break;
              }
            }
            if (*(sp+1) == 'S' || *(sp+1) == 's')
              sp++;
          } else {
            do_print(buf, dms(x[3], round_flag));
          }
          break;
      case 'B':
          do_print(buf, dms(x[1], round_flag));
          break;
      case 'b':
          sprintf(s, "%# 11.7f", x[1]);
          do_print(buf, s);
          break;
      case 'A': /* rectascensio */
          do_print(buf, dms(xequ[0]/15, round_flag|SEFLG_EQUATORIAL));
          break;
      case 'a': /* rectascensio */
          sprintf(s, "%# 11.7f", xequ[0]);
          do_print(buf, s);
          break;
      case 'D': /* declination */
          do_print(buf, dms(xequ[1], round_flag));
          break;
      case 'd': /* declination */
          sprintf(s, "%# 11.7f", xequ[1]);
          do_print(buf, s);
          break;
      case 'R':
          sprintf(s, "%# 14.9f", x[2]);
          do_print(buf, s);
          break;
      case 'r':
          if ( ipl == SE_MOON ) { /* for moon print parallax */
            sinp = 8.794 / x[2];        /* in seconds of arc */
            ar = sinp * (1 + sinp * sinp * 3.917402e-12);
            /* the factor is 1 / (3600^2 * (180/pi)^2 * 6) */
            sprintf(s, "%# 13.5f\"", ar);       
          } else {
            sprintf(s, "%# 14.9f", x[2]);
          }
          do_print(buf, s);
          break;
      case 'U':
      case 'X':
          if (*sp =='U') 
            ar = sqrt(square_sum(xcart));
          else 
            ar = 1;
          sprintf(s, "%# 14.9f%s", xcart[0]/ar, gap);
          do_print(buf, s);
          sprintf(s, "%# 14.9f%s", xcart[1]/ar, gap);
          do_print(buf, s);
          sprintf(s, "%# 14.9f", xcart[2]/ar);
          do_print(buf, s);
          break;
      case 'u':
      case 'x':
          if (*sp =='u') 
            ar = sqrt(square_sum(xcartq));
          else 
            ar = 1;
          sprintf(s, "%# 14.9f%s", xcartq[0]/ar, gap);
          do_print(buf, s);
          sprintf(s, "%# 14.9f%s", xcartq[1]/ar, gap);
          do_print(buf, s);
          sprintf(s, "%# 14.9f", xcartq[2]/ar);
          do_print(buf, s);
          break;
      case 'Q':
          sprintf(s, "%-15s", spnam);
          do_print(buf, s);
          do_print(buf, dms(x[0], round_flag));
          do_print(buf, dms(x[1], round_flag));
          sprintf(s, "  %# 14.9f", x[2]);
          do_print(buf, s);
          do_print(buf, dms(x[3], round_flag));
          do_print(buf, dms(x[4], round_flag));
          sprintf(s, "  %# 14.9f\n", x[5]);
          do_print(buf, s);
          sprintf(s, "               %s", dms(xequ[0], round_flag));
          do_print(buf, s);
          do_print(buf, dms(xequ[1], round_flag));
          sprintf(s, "                %s", dms(xequ[3], round_flag));
          do_print(buf, s);
          do_print(buf, dms(xequ[4], round_flag));
          break;
      } /* switch */
    }   /* for sp */
    if (calc_house_pos) {
      sprintf(s, "  %# 6.4f", hpos);
          sprintf(s, "%# 9.4f", hpos);
      do_print(buf, s);
    }
    do_print(buf, "\n");
  }     /* for psp */
  if (*serr_warn != '\0') {
    do_print(buf, "\nwarning: ");
    do_print(buf, serr_warn);
    do_print(buf, "\n");
  }
  /* houses */
  sprintf(s, "\nHouse Cusps (%s)\n\n", pd.hsysname);
  do_print(buf, s);
  a = sidt + 0.5 / 3600;
  sprintf(s, "sid. time : %4d:%02d:%02d  ",
        (int) a, (int) fmod(a * 60, 60), (int) fmod(a * 3600, 60));
  do_print(buf, s);
  a = armc + 0.5 / 3600;
  sprintf(s, "armc      : %4d%s%02d'%02d\"\n",
        (int) armc, MY_ODEGREE_STRING, (int) fmod(armc * 60, 60),
        (int) fmod(a * 3600, 60));
  do_print(buf, s);
  sprintf(s, "geo. lat. : %4d%c%02d'%02d\" ",
        pd.lat_deg, *pd.lat_n_s, pd.lat_min, pd.lat_sec);
  do_print(buf, s);
  sprintf(s, "geo. long.: %4d%c%02d'%02d\"\n\n",
        pd.lon_deg, *pd.lon_e_w, pd.lon_min, pd.lon_sec);
  do_print(buf, s);
  swe_houses_ex(tjd_ut, iflag, lat, lon, hsys, cusp, ascmc);
  round_flag |= BIT_ROUND_SEC;
#if FALSE
  sprintf(s, "AC        : %s\n", dms(ascmc[0], round_flag));
  do_print(buf, s);
  sprintf(s, "MC        : %s\n", dms(ascmc[1], round_flag));
  do_print(buf, s);
  for (i = 1; i <= 12; i++) {
	sprintf(s, "house   %2d: %s\n", i, dms(cusp[i], round_flag));
    do_print(buf, s);
  }
  sprintf(s, "Vertex    : %s\n", dms(ascmc[3], round_flag));
  do_print(buf, s);
#else
  sprintf(s, "AC        : %s\n", dms(ascmc[0], round_flag|BIT_ZODIAC));
  do_print(buf, s);
  sprintf(s, "MC        : %s\n", dms(ascmc[1], round_flag|BIT_ZODIAC));
  do_print(buf, s);
  for (i = 1; i <= 12; i++) {
	sprintf(s, "house   %2d: %s\n", i, dms(cusp[i], round_flag|BIT_ZODIAC));
    do_print(buf, s);
  }
  sprintf(s, "Vertex    : %s\n", dms(ascmc[3], round_flag|BIT_ZODIAC));
  do_print(buf, s);
#endif  
  return 0;
}
 
static char *dms(double x, long iflag)
{
  int izod;
  long k, kdeg, kmin, ksec;
  char *c = MY_ODEGREE_STRING;
  char *sp, s1[50];
  static char s[50];
  int sgn;
  *s = '\0';
  if (iflag & SEFLG_EQUATORIAL)
    strcpy(c, "h");
  if (x < 0) {
    x = -x;
    sgn = -1;
  } else 
    sgn = 1;
  if (iflag & BIT_ROUND_MIN)
    x += 0.5/60;
  if (iflag & BIT_ROUND_SEC)
    x += 0.5/3600;
  if (iflag & BIT_ZODIAC) {
    izod = (int) (x / 30); 
    x = fmod(x, 30);
    kdeg = (long) x;
    sprintf(s, "%2ld %s ", kdeg, zod_nam[izod]);
  } else {
    kdeg = (long) x;
    sprintf(s, " %3ld%s", kdeg, c);
  }
  x -= kdeg;
  x *= 60;
  kmin = (long) x;
  if ((iflag & BIT_ZODIAC) && (iflag & BIT_ROUND_MIN))
    sprintf(s1, "%2ld", kmin);
  else
    sprintf(s1, "%2ld'", kmin);
  strcat(s, s1);
  if (iflag & BIT_ROUND_MIN)
    goto return_dms;
  x -= kmin;
  x *= 60;
  ksec = (long) x;
  if (iflag & BIT_ROUND_SEC)
    sprintf(s1, "%2ld\"", ksec);
  else
    sprintf(s1, "%2ld", ksec);
  strcat(s, s1);
  if (iflag & BIT_ROUND_SEC)
    goto return_dms;
  x -= ksec;
  k = (long) (x * 10000);
  sprintf(s1, ".%04ld", k);
  strcat(s, s1);
  return_dms:;
  if (sgn < 0) {
    sp = strpbrk(s, "0123456789"); 
    *(sp-1) = '-';
  }
  return(s);
}

static void do_print(char *target, char *info)
{
  char *sp;
  if (*target == '\0')
    strcpy(target, " ");
  for (sp = info; *sp != '\0'; sp++) {
    if (*sp == '\n') 
      strcat(target, "\r\n");
    else
      strncat(target, sp, 1);
  }
}

static int letter_to_ipl(int letter)
{
  if (letter >= '0' && letter <= '9')
    return letter - '0' + SE_SUN;
  if (letter >= 'A' && letter <= 'I')
    return letter - 'A' + SE_MEAN_APOG;
  if (letter >= 'J' && letter <= 'X')
    return letter - 'J' + SE_CUPIDO;
  switch (letter) {
  case 'm': return SE_MEAN_NODE;
  case 'n': 
  case 'o': return SE_ECL_NUT;
  case 't': return SE_TRUE_NODE;
  case 'f': return SE_FIXSTAR;
  }
  return -1;
}

static int atoulng(char *s, unsigned long *lng)
{
  char *sp;
  for (sp = s; *sp != '\0'; sp++) {
    if (!isdigit(*sp)) {
      *lng = 0;
      return ERR;
    }
  }
  *lng = atol(s);
  return OK;
}

static int atoslng(char *s, long *lng)
{
  char *sp;
  for (sp = s; *sp != '\0'; sp++) {
    if (!isdigit(*sp) && *sp != '-') {
      *lng = 0;
      return ERR;
    }
  }
  *lng = atol(s);
  return OK;
}

/* make_ephemeris_path().
 * ephemeris path includes
 *   current working directory
 *   + program directory
 *   + default path from swephexp.h on current drive
 *   +                              on program drive
 *   +                              on drive C:
 */
static int make_ephemeris_path(long iflag, char *argv0)
{
  char path[AS_MAXCH], s[AS_MAXCH];
  char *sp;
  char *dirglue = DIR_GLUE;
  size_t pathlen; 
  /* moshier needs no ephemeris path */
  if (iflag & SEFLG_MOSEPH)
    return OK;
  /* current working directory */ 
  sprintf(path, ".%c", *PATH_SEPARATOR);
  /* program directory */
  sp = strrchr(argv0, *dirglue);
  if (sp != NULL) {
    pathlen = sp - argv0;
    if (strlen(path) + pathlen < AS_MAXCH-1) {
      strcpy(s, argv0);
      *(s+pathlen) = '\0';
      sprintf(path + strlen(path), "%s%c", s, *PATH_SEPARATOR);
    } 
  }
#if MSDOS
{
  char *cpos[20];
  char s[2 * AS_MAXCH], *s1 = s + AS_MAXCH;
  char *sp[3];
  int i, j, np;
  strcpy(s1, SE_EPHE_PATH);
  np = cut_str_any(s1, PATH_SEPARATOR, cpos, 20);
  /* 
   * default path from swephexp.h
   * - current drive
   * - program drive
   * - drive C
   */
  *s = '\0';
  /* current working drive */
  sp[0] = getcwd(NULL, 0);
  if (sp[0] == NULL) {
    /*do_printf("error in getcwd()\n");*/
    return ERR;
  } 
  if (*sp[0] == 'C')
    sp[0] = NULL;
  /* program drive */
  if (*argv0 != 'C' && (sp[0] == NULL || *sp[0] != *argv0))
    sp[1] = argv0;
  else
    sp[1] = NULL;
    /* drive C */
  sp[2] = "C";
  for (i = 0; i < np; i++) {
    strcpy(s, cpos[i]);
    if (*s == '.')	/* current directory */
      continue;
    if (s[1] == ':')  /* drive already there */
      continue;
    for (j = 0; j < 3; j++) {
      if (sp[j] != NULL && strlen(path) + 2 + strlen(s) < AS_MAXCH-1)
	sprintf(path + strlen(path), "%c:%s%c", *sp[j], s, *PATH_SEPARATOR);
    }
  }
}
#else
    if (strlen(path) + pathlen < AS_MAXCH-1)
      strcat(path, SE_EPHE_PATH);
#endif
  return OK;
}

/**************************************************************
cut the string s at any char in cutlist; put pointers to partial strings
into cpos[0..n-1], return number of partial strings;
if less than nmax fields are found, the first empty pointer is
set to NULL.
More than one character of cutlist in direct sequence count as one
separator only! cut_str_any("word,,,word2",","..) cuts only two parts,
cpos[0] = "word" and cpos[1] = "word2".
If more than nmax fields are found, nmax is returned and the
last field nmax-1 rmains un-cut.
**************************************************************/
static int cut_str_any(char *s, char *cutlist, char *cpos[], int nmax)
{
  int n = 1;
  cpos [0] = s;
  while (*s != '\0') {
    if ((strchr(cutlist, (int) *s) != NULL) && n < nmax) {
      *s = '\0';
      while (*(s + 1) != '\0' && strchr (cutlist, (int) *(s + 1)) != NULL) s++;
      cpos[n++] = s + 1;
    }
    if (*s == '\n' || *s == '\r') {	/* treat nl or cr like end of string */
      *s = '\0';
      break;
    }
    s++;
  }
  if (n < nmax) cpos[n] = NULL;
  return (n);
}	/* cutstr */
