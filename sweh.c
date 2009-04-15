#include "swephexp.h"
#include "sweph.h"
#include "swephlib.h"
#include <sys/stat.h>
/*
int32 FAR PASCAL_CONV swe_heliacal_ut(double JDNDaysUTStart, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 TypeEvent, int32 iflag, double *dret, char *serr);
int32 FAR PASCAL_CONV swe_heliacal_pheno_ut(double JDNDaysUT, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 TypeEvent, int32 helflag, double *darr, char *serr);
int32 FAR PASCAL_CONV swe_vis_limit_mag(double tjdut, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 helflag, double *dret, char *serr);
/* the following two are secret, for Victor Reijs' test environment */

#define MERCURY_DAYSTEP 		64

/*
 *  swehel -b1.1.2000 -xfvenus -geopos8,47,400 -at1000,15,15,0.2 -obs23,1 -e1 -hp -n1
 */
int main(int argc, char *argv[])
{
  int i;
  char starname[AS_MAXCH], serr[AS_MAXCH];
  char *begindate = "4.6.1959", sdate[20], *sp, *sp2;
  double dgeo[3], dret[40], datm[4];
  double dobs[6];
  double Age = 42, SN = 1;
  double jut, tjd, tjdh, tjdhs = 0, tstep = 0;
  int istep, nstep = 10;
  double Temperature = 0, Pressure = 1013, RH = 50, VR = 0;
  int jyear, jmon, jday, gregflag, TypeEvent = 1, iflag = 0, iflagret, helflag=SEFLG_SWIEPH;
  double x[6];
  AS_BOOL call_vislim = FALSE, make_list = FALSE;
  AS_BOOL is_testrun = FALSE, print_vbcall = FALSE, call_pheno = FALSE;
  /* optic stuff */
  dobs[2] = 0; /* 1-binocular 0=monocular*/
  dobs[3] = 0; /*telescope magnification*/
  dobs[4] = 0; /*telescope diameter [mm]*/
  dobs[5] = 0; /*telescope transmission*/
  for (i = 1; i < argc; i++) {
    if (strncmp(argv[i], "-te", 3) == 0) {
      Temperature = atof(argv[i]+3);
    } else if (strncmp(argv[i], "-ag", 3) == 0) {
      Age = atof(argv[i]+3);
      dobs[0] = Age;
    } else if (strncmp(argv[i], "-sn", 3) == 0) {
      SN = atof(argv[i]+3);
      dobs[1] = SN;
#if 0
    } else if (strncmp(argv[i], "-opt", 4) == 0) {
      dobs[2] = 1; /* 1-binocular 0=monocular*/
      dobs[3] = 2; /*telescope magnification*/
      dobs[4] = 10; /*telescope diameter [mm]*/
      dobs[5] = 1; /*telescope transmission*/
#endif
    } else if (strncmp(argv[i], "-at", 3) == 0) {
      sscanf(argv[i]+3, "%lf,%lf,%lf,%lf", &Pressure, &Temperature, &RH, &VR);
    } else if (strncmp(argv[i], "-obs", 4) == 0) {
      helflag |= SE_HELFLAG_OPTICAL_PARAMS;
      sscanf(argv[i]+4, "%lf,%lf,%lf,%lf,%lf,%lf", &(dobs[0]), &(dobs[1]), &(dobs[2]), &(dobs[3]), &(dobs[4]), &(dobs[5]));
    } else if (strncmp(argv[i], "-t", 2) == 0) {
      is_testrun = TRUE;
    } else if (strncmp(argv[i], "-b", 2) == 0) {
      begindate = argv[i] + 2;
    } else if (strncmp(argv[i], "-mkl", 4) == 0) {
      make_list = TRUE;
    } else if (strncmp(argv[i], "-ph", 3) == 0) {
      call_pheno = TRUE;
    } else if (strncmp(argv[i], "-nodet", 3) == 0) {
      helflag |= SE_HELFLAG_NO_DETAILS;
    } else if (strncmp(argv[i], "-n", 2) == 0) {
      nstep = atoi(argv[i]+2);
      if (nstep == 0)
        nstep = 20;
    } else if (strncmp(argv[i], "-e", 2) == 0) {
      TypeEvent = atof(argv[i]+2);
    } else if (strncmp(argv[i], "-s", 2) == 0) {
      tstep = atof(argv[i]+2);
    } else if (strncmp(argv[i], "-rh", 3) == 0) {
      RH = atof(argv[i]+3);
    } else if (strncmp(argv[i], "-vislim", 7) == 0) {
      call_vislim = TRUE;
    } else if (strncmp(argv[i], "-vr", 3) == 0) {
      VR = atof(argv[i]+3);
    } else if (strncmp(argv[i], "-v", 2) == 0) {
      print_vbcall = TRUE;
    } else if (strncmp(argv[i], "-hp", 3) == 0) {
      helflag |= SE_HELFLAG_HIGH_PRECISION;
    } else if (strncmp(argv[i], "-ls", 3) == 0) {
      helflag |= SE_HELFLAG_LONG_SEARCH;
    } else if (strncmp(argv[i], "-geopos", 7) == 0) {
      sscanf(argv[i] + 7, "%lf,%lf,%lf", &(dgeo[0]), &(dgeo[1]), &(dgeo[2]));
    } else if (strncmp(argv[i], "-av", 3) == 0) {
      helflag |= atoi(argv[i]+3);
    } else if (strncmp(argv[i], "-xf", 3) == 0) {
      /* name or number of fixed star */
      strcpy(starname, argv[i] + 3);
    }
  }
  //for (i = 0; i < 6; i++) printf("dobs[%d] = %f\n", i, dobs[i]);
  datm[0] = Pressure;
  datm[1] = Temperature;
  datm[2] = RH;
  datm[3] = VR;
  if (!is_testrun) {
    strcpy(sdate, begindate);
    sp = sdate;
    if (*sp == 'j') {   /* it's a day number */
      if ((sp2 = strchr(sp, ',')) != NULL)
        *sp2 = '.';
      sscanf(sp+1,"%lf", &tjd);
      if (tjd < 2299160.5)
        gregflag = SE_JUL_CAL;
      else
        gregflag = SE_GREG_CAL;
      if (strstr(sp, "jul") != NULL)
        gregflag = SE_JUL_CAL;
      else if (strstr(sp, "greg") != NULL)
        gregflag = SE_GREG_CAL;
      swe_revjul(tjd, gregflag, &jyear, &jmon, &jday, &jut);
    } else {
      if (sscanf (sp, "%d%*c%d%*c%d", &jday,&jmon,&jyear) < 1) exit(1);
      if ((int32) jyear * 10000L + (int32) jmon * 100L + (int32) jday < 15821015L)
        gregflag = SE_JUL_CAL;
      else
        gregflag = SE_GREG_CAL;
      if (strstr(sp, "jul") != NULL)
        gregflag = SE_JUL_CAL;
      else if (strstr(sp, "greg") != NULL)
        gregflag = SE_GREG_CAL;
      jut = 0;
      tjd = swe_julday(jyear,jmon,jday,jut,gregflag);
    }
#if 0
HeliacalJDut(1750960.5,23,1,22.5,30.7,60,20,1029,10,120,"mizar",1,AVkind,dret,serr);
printf("%f %s\n", *dret, serr);
exit(0);
#endif
#if 0
if (0) {
int32 ipl = SE_SUN;
int32 TypeEvent = SE_CALC_SET|SE_BIT_DISC_CENTER;
Temperature = 10;
calc_rise_and_set(tjd, ipl, dgeo, Pressure, Temperature, iflag, TypeEvent, helflag, dret, serr);
printf("1: %f\n", dret[0]);
swe_rise_trans(tjd, ipl, "", iflag, TypeEvent, dgeo, Pressure, Temperature, dret, serr);
printf("2: %f\n", dret[0]);
exit(0);
}
#endif
/*calc_planet_fast(tjd, SE_MERCURY, dret);
printf("tjd = %f, l = %f, b = %f, r = %f\n", tjd, dret[0], dret[1], dret[2]);
exit (0);*/
    if (call_pheno) {
      *serr = '\0';
      iflagret = swe_heliacal_pheno_ut(tjd, dgeo, datm, dobs, starname, TypeEvent, helflag, dret, serr);
      if (serr != '\0')
	printf("%s\n", serr);
      for (i = 0; i <= 30; i++)
	printf("%d\t%.10f\n", i, dret[i]);
    } else if (make_list) {
      int32 ipl, iev;
      double tjd0 = tjd;
      double tend = tjd + 366;
      for (ipl = SE_MOON; ipl <= SE_SATURN; ipl++) {
	strcpy(starname, swe_get_planet_name(ipl, starname));
        for (iev = 1; iev <= 4; iev++) {
	  if (ipl == SE_MOON && iev <= 2)
	    continue;
	  if (ipl >= SE_MARS && iev >= 3)
	    continue;
	  tjd = tjd0;
	  while (tjd < tend) {
	    iflagret = swe_heliacal_ut(tjd, dgeo, datm, dobs, starname, iev, helflag, &tjd, serr);
	    if (iflagret == ERR) {
	      printf("%s\n", serr);
	      break;
	    }
	    swe_revjul(tjd, SE_GREG_CAL, &jyear, &jmon, &jday, &jut);
	    if (tjd > tend) 
	      break;
	    printf("ipl=%s, event=%d, date=%d.%d.%d (%f)\n", starname, iev, jday, jmon, jyear, tjd);
	    if (ipl >= SE_VENUS)
	      break;
	    tjd += 10;
	  }
	}
      }
    } else if (call_vislim) {
      double tjdi;
      double dt = 5.0 / 60.0 / 24.0;
      int j = 289;
      if (nstep <= 1) j = 1;
      if (nstep == 1000) {
        j = 1440;
	dt = 1.0 / 1440.0;
      }
      *serr = '\0';
      for (i = 0; i < j; i++) {
        char *is_visible = "";
	tjdi = tjd + i * dt;
	iflagret = swe_vis_limit_mag(tjdi, dgeo, datm, dobs, starname, helflag, dret, serr);
	if (dret[0] > dret[7]) 
	  is_visible = "vis";
	if (iflagret != ERR) {
if (0) {
	  int k;
	  for (k = 0; k < 3; k++) printf("dgeo[%d] = %f\n", k, dgeo[k]);
	  for (k = 0; k < 4; k++) printf("datm[%d] = %f\n", k, datm[k]);
	  for (k = 0; k < 6; k++) printf("dobs[%d] = %f\n", k, dobs[k]);
	  printf("swe_vis_limit_mag(%f, dgeo, datm, dobs, %s, %d, dret, serr)\n", tjdi, starname, helflag);
}
	  printf("vislim=%.17f mO=%f (%f), altO=%f, altS=%f, sc/ph=%1.0f %s\n", dret[0], dret[7], tjdi, dret[1], dret[3], (double) iflagret, is_visible);
	}
	if (serr != '\0' && (i == 0 || i == 288))
	  printf("%s\n", serr);
      }
    } else {
      double tjdsv;
      for (istep = 0; istep < nstep; istep++) {
	if (print_vbcall)
	  printf("HeliacalJDut(%f,%f,1,%f,%f,%f,%f,%f,%f,%f,\"%s\",%d)\n", tjd, dobs[0], dgeo[1], dgeo[0], dgeo[2], Temperature, Pressure, RH, VR, starname, TypeEvent);
	*serr = '\0';
	tjdsv = tjd;
	iflagret = swe_heliacal_ut(tjd, dgeo, datm, dobs, starname, TypeEvent, helflag, dret, serr);
	tjdh = tjd;
	if (iflagret == OK) {
	  tjdh = dret[0];
	}
	if (tstep == 1) {
	  tjd += 1;
	} else if (strstr(starname, "oon") != NULL) {
	  tjd = tjdh + 25;
	} else if (strstr(starname, "ercury") != NULL) {
	  tjd = tjdh + 30;
	} else {
	  tjd = tjdh + 30;
	}
	if (iflagret < 0) {
	  if (serr != NULL && *serr != '\0')
	    printf("%s\n", serr);
	  continue;
	}
	iflagret = swe_calc(tjdh, SE_SUN, iflag, x, NULL);
	if (iflagret == ERR) {
	  if (serr != NULL && *serr != '\0')
	    printf("%s\n", serr);
	  exit(0);
	}
	swe_revjul(tjdh, SE_GREG_CAL, &jyear, &jmon, &jday, &jut);
	//printf("tjdh = %.10f\t%d/%d/%d\tsun = %f\t%f\n", tjdh, jyear, jmon, jday, x[0], tjdh - tjdhs);
	//printf("%f->%f,%f,%f,%f,%f,%f\t%d/%d/%d\tsun = %f\t%f\n", tjdsv, dret[0],dret[1],dret[2],dret[3],dret[4],dret[5], jyear, jmon, jday, x[0], tjdh - tjdhs);
	printf("%f,%f,%f\t%d/%d/%d\tsun = %f\t%f\n", dret[0],dret[1],dret[2], jyear, jmon, jday, x[0], tjdh - tjdhs);
	tjdhs = tjdh;
	if (*serr != '\0')
	  printf("%s\n", serr);
      }
    }
  }
#if 1
  if (is_testrun) {
    /*HeliacalAngle(2.75, 42, 1, 79.53, -7, 58, 625928.5663, 76.63, 32.34, 60, 0, 1013, 0, 0.16, dret);
    printf("%f\n", dret[0]);
    HeliacalAngle(-2.75, 42, 1, 79.53, -7, 58, 625928.5663, 76.63, 32.34, 60, 0, 1013, 0, 0.16, dret);
    printf("%f\n", dret[0]);
    HeliacalAngle(-2.75, 42, 1, 79.53, -7, 58, 625928.5663, 76.63, 32.34, 60, 0, 1013, 0, 0.16, dret);
    printf("%f\n", dret[1]);*/
    /* iflagret = swe_heliacal_ut(tjd, dgeo, Temperature, Pressure, RH, VR, Age, 1, starname, TypeEvent, helflag, dret, serr);*/
    strcpy(starname, "moon");
    dgeo[0] = 23.7; dgeo[1] = 38.0; dgeo[2] = 100;
    datm[0] = 1013; datm[1] = 0; datm[2] = 0; datm[3] = 100;
    dobs[0] = 42; dobs[1] = 1;
    iflagret = swe_heliacal_ut(2402030.5,dgeo,datm,dobs,starname,3,helflag, dret, serr);
    printf("%f\n", dret[0]);
    swe_heliacal_pheno_ut(dret[0], dgeo,datm,dobs,starname,3,helflag, dret,serr);
    for (i = 0; i < 26; i++)
      printf("%d\t%f\n", i, dret[i]);
    if (*serr != '\0')
      printf(serr);
    strcpy(starname, "venus");
    dgeo[0] = 23.7; dgeo[1] = 38.0; dgeo[2] = 100;
    iflagret = swe_heliacal_ut(2402030.5,dgeo,datm,dobs,starname,3,helflag, dret, serr);
    printf("%f\n", dret[0]);
    swe_heliacal_pheno_ut(dret[0], dgeo,datm,dobs,starname,3,helflag, dret,serr);
    for (i = 0; i < 26; i++)
      printf("%d\t%f\n", i, dret[i]);
    if (*serr != '\0')
      printf(serr);
    strcpy(starname, "sirius");
    dgeo[0] = 30.7; dgeo[1] = 22.5; dgeo[2] = 60;
    datm[0] = 1029; datm[1] = 20; datm[2] = 10; datm[3] = 120;
    dobs[0] = 23; dobs[1] = 1;
    helflag = SE_HELFLAG_HIGH_PRECISION;
    iflagret = swe_heliacal_pheno_ut(1241083.66657298,dgeo,datm,dobs,starname,3,helflag, dret, serr);
    for (i = 0; i < 26; i++)
      printf("%d\t%f\n", i, dret[i]);
    if (*serr != '\0')
      printf(serr);
    exit(0);
    strcpy(starname, "sirius");
    dgeo[0] = 30.7; dgeo[1] = 22.5; dgeo[2] = 60;
    datm[0] = 1029; datm[1] = 20; datm[2] = 10; datm[3] = 120;
    iflagret = swe_heliacal_ut(1241083.66657298,dgeo,datm,dobs,starname,3,helflag, dret, serr);
    printf("%f\n", dret[0]);
  }
#endif
  return 0;
}
