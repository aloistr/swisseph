/* Automatically generated file - do not change manually */
/* Create or change source files named suite_*.c         */
/* Generate this file using "make"                       */

// This source file (i.e. globals_suite.c) contains utility functions for writing test case code

#include "testsuite_final.h"

void check_swecalc_results(int rc,double *xx, char* serr, test_context* ctx) {
  check_dd(xx,6,"xx",ctx);
  check_i(rc,"rc",ctx);
  check_s(serr,"serr",ctx);
}

void check_swehouses_results(int rc, double jd_ut, int ihsy, double *cusps, double *ascmc, char* serr, test_context* ctx) {
  check_d(jd_ut,"jd_ut",ctx);
  if (ihsy == 'G') {
    check_dd(cusps,37,"cusps",ctx);
  } else {
    check_dd(cusps,13,"cusps",ctx);
  }
  check_dd(ascmc,6,"ascmc",ctx);	// 6 = do not test coasc2, 8 = all 
  check_i(rc,"rc",ctx);
}

void check_swehouses_ex2_results(int rc, double jd_ut, int ihsy, double *cusps, double *ascmc, double *cusp_speed, double *ascmc_speed, char* serr, test_context* ctx) {
  check_d(jd_ut,"jd_ut",ctx);
  if (ihsy == 'G') {
    check_dd(cusps,37,"cusps",ctx);
    check_dd(cusp_speed,37,"cusp_speed",ctx);
  } else {
    check_dd(cusps,13,"cusps",ctx);
    check_dd(cusp_speed,13,"cusp_speed",ctx);
  }
  check_dd(ascmc,6,"ascmc",ctx);	// 6 = do not test coasc2, 8 = all 
  check_dd(ascmc_speed,6,"ascmc_speed",ctx);	// 6 = do not test coasc2, 8 = all 
  check_i(rc,"rc",ctx);
}

void check_swehouses_armc_results(int rc, double armc, int ihsy, double *cusps, double *ascmc, char* serr, test_context* ctx) {
  check_d(armc,"armc",ctx);
  if (ihsy == 'G') {
    check_dd(cusps,37,"cusps",ctx);
  } else {
    check_dd(cusps,13,"cusps",ctx);
  }
  check_dd(ascmc,6,"ascmc",ctx);	// 6 = do not test coasc2, 8 = all 
  check_i(rc,"rc",ctx);
}

void check_swehouses_armc_ex2_results(int rc, double armc, int ihsy, double *cusps, double *ascmc, double *cusp_speed, double *ascmc_speed, char* serr, test_context* ctx) {
  check_d(armc,"armc",ctx);
  if (ihsy == 'G') {
    check_dd(cusps,37,"cusps",ctx);
    check_dd(cusp_speed,37,"cusp_speed",ctx);
  } else {
    check_dd(cusps,13,"cusps",ctx);
    check_dd(cusp_speed,13,"cusp_speed",ctx);
  }
  check_dd(ascmc,6,"ascmc",ctx);	// 6 = do not test coasc2, 8 = all 
  check_dd(ascmc_speed,6,"ascmc_speed",ctx);	// 6 = do not test coasc2, 8 = all 
  check_i(rc,"rc",ctx);
}

// End of source file globals_suite.c

#include "testsuite_final.h"



void testsuite_1(test_context *ctx) {

double xx[6],jd;
int iflag, iephe; // Keep ephemeris selector separate from other flags
char serr[255];

swe_set_ephe_path(NULL);
swe_set_jpl_file("de431.eph");

void testcase_1_1(test_context *ctx) {
  int rc = swe_calc(jd, get_i("ipl",ctx), iflag | iephe, xx, serr);
  check_swecalc_results(rc,xx,serr,ctx);
  }

void testcase_1_2(test_context *ctx) {
  int ipl = get_i("ipl",ctx); 
swe_set_ephe_path(NULL);
  int rc = swe_calc_ut(jd, ipl, iflag | iephe, xx, serr);
  check_swecalc_results(rc,xx,serr,ctx);
  }

void testcase_1_3(test_context *ctx) {
  double geolon = get_d("geolon",ctx),
         geolat = get_d("geolat",ctx),
         altitude = get_d("altitude",ctx);
swe_set_ephe_path(NULL);
  int ipl = get_i("ipl",ctx);
  swe_set_topo( geolon, geolat, altitude );
  int rc = swe_calc(jd, ipl, iflag | iephe, xx, serr);
  check_swecalc_results(rc,xx,serr,ctx);
}

void testcase_1_4(test_context *ctx) {
  iflag = SEFLG_EQUATORIAL | iephe;
  int ipl = get_i("ipl",ctx);
swe_set_ephe_path(NULL);
  swe_calc(jd, ipl, iflag, xx, serr);
  iflag =  iephe;
  int rc = swe_calc(jd, ipl, iflag, xx, serr);
  check_equals_i(rc,iflag,"rc",ctx); // we expect rc = iflag = iephe
  }

void testcase_1_5(test_context *ctx) {
  int rc = swe_calc_pctr(jd, get_i("ipl",ctx), get_i("iplctr",ctx), iflag | iephe, xx, serr);
  check_swecalc_results(rc,xx,serr,ctx);
  }

void setup_1() {
  iflag = get_i("iflag",ctx);
  iephe = get_i("iephe",ctx);
  jd = get_d("jd",ctx);
  *serr = '\0';
  }

void teardown_1() {
  if (get_i("initialize",ctx)) swe_close( );
  }


  open_testsuite(1,"Various swe_calc calls in different modes",ctx);


  if (is_testcase_selected( 1, ctx)) { 
    open_testcase(1,"swe_calc( ) - Julian Date with Ephemeris Time",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_1();
        testcase_1_1(ctx);
        teardown_1();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 2, ctx)) { 
    open_testcase(2,"swe_calc_ut( ) - Julian Date with Universal Time",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_1();
        testcase_1_2(ctx);
        teardown_1();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 3, ctx)) { 
    open_testcase(3,"swe_set_topo( ) - Using topographic coordinates",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_1();
        testcase_1_3(ctx);
        teardown_1();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 4, ctx)) { 
    open_testcase(4,"swe_calc( ) - Equatorial followed by Ecliptic",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_1();
        testcase_1_4(ctx);
        teardown_1();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 5, ctx)) { 
    open_testcase(5,"swe_calc_pctr( ) - planetocentic position",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_1();
        testcase_1_5(ctx);
        teardown_1();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }

}
#include "testsuite_final.h"


void testsuite_2(test_context *ctx) {

double xx[6],jd;
int rc, iflag, ipl;
char serr[255], star[100];

// swe_fixstar doesn't read env.var. SE_EPHE_PATH
// swe_calc does. So for this suite, we have to read it explicitly
swe_set_ephe_path(NULL);


void testcase_2_1(test_context *ctx) {

  jd = get_d("jd",ctx);
  iflag = get_i("iflag",ctx);
  strcpy(star,get_s("star",ctx));

  rc = swe_fixstar(star,jd,iflag,xx,serr);

  check_swecalc_results(rc,xx,serr,ctx);

  }

void testcase_2_2(test_context *ctx) {

  jd = get_d("jd",ctx);
  strcpy(star,get_s("star",ctx));

  rc = swe_fixstar_ut(star,jd,iflag,xx,serr);

  check_swecalc_results(rc,xx,serr,ctx);

  }

void testcase_2_3(test_context *ctx) {
  double mag;

  strcpy(star,get_s("star",ctx));
  rc = swe_fixstar_mag(star,&mag,serr);

  check_d(mag,"mag",ctx);
  check_i(rc,"rc",ctx);
  check_s(serr,"serr",ctx); 

  }

void testcase_2_4(test_context *ctx) {
  swe_set_ephe_path(NULL);
  jd = get_d("jd",ctx);
  ipl = 2;
  strcpy(star,get_s("star",ctx));
  rc = swe_calc(jd, ipl, iflag, xx, serr);
  rc = swe_fixstar(star,jd,iflag,xx,serr);

  check_swecalc_results(rc,xx,serr,ctx);

  }

void testcase_2_5(test_context *ctx) {
  swe_set_ephe_path(NULL);
  jd = get_d("jd",ctx);
  ipl = 2;
  strcpy(star, "Algol");
  rc = swe_fixstar(star,jd,iflag,xx,serr);
  strcpy(star,get_s("star",ctx));
  rc = swe_fixstar(star,jd,iflag,xx,serr);

  check_swecalc_results(rc,xx,serr,ctx);

  }

void testcase_2_6(test_context *ctx) {

  swe_set_ephe_path(NULL);
  jd = get_d("jd",ctx);
  iflag = get_i("iflag",ctx);
  strcpy(star,get_s("star",ctx));

  rc = swe_fixstar2(star,jd,iflag,xx,serr);

  check_swecalc_results(rc,xx,serr,ctx);

  }

void testcase_2_7(test_context *ctx) {

  swe_set_ephe_path(NULL);
  jd = get_d("jd",ctx);
  strcpy(star,get_s("star",ctx));

  rc = swe_fixstar2_ut(star,jd,iflag,xx,serr);

  check_swecalc_results(rc,xx,serr,ctx);

  }


  open_testsuite(2,"Fixed stars",ctx);


  if (is_testcase_selected( 1, ctx)) { 
    open_testcase(1,"Fixed stars with Ephemeris Time",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        
        testcase_2_1(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 2, ctx)) { 
    open_testcase(2,"Fixed stars with Universal Time",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        
        testcase_2_2(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 3, ctx)) { 
    open_testcase(3,"Magnitude of a fixed star",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        
        testcase_2_3(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 4, ctx)) { 
    open_testcase(4,"Mercury, then Betelgeuze",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        
        testcase_2_4(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 5, ctx)) { 
    open_testcase(5,"Algol, then Betelgeuze",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        
        testcase_2_5(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 6, ctx)) { 
    open_testcase(6,"Fixed stars (swe_fixstar2) with Ephemeris Time",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        
        testcase_2_6(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 7, ctx)) { 
    open_testcase(7,"Fixed stars (swe_fixstar2) with Universal Time",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        
        testcase_2_7(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }

}
#include "testsuite_final.h"


void testsuite_3(test_context *ctx) {

void testcase_3_1(test_context *ctx) {
  int ipl = get_i("ipl",ctx);
  char name[255];
  swe_get_planet_name(ipl,name);
  check_s(name,"name",ctx);
}

void testcase_3_2(test_context *ctx) {
  int sid_mode = get_i("sid_mode",ctx);
  const char *name = swe_get_ayanamsa_name(sid_mode);
  check_s(name,"name",ctx);
}



  open_testsuite(3,"Various small functions (names et al.)",ctx);


  if (is_testcase_selected( 1, ctx)) { 
    open_testcase(1,"swe_get_planet_name( ) - Getting a planet name",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        
        testcase_3_1(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 2, ctx)) { 
    open_testcase(2,"swe_get_aya_name( ) - Getting an ayanamsa name",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        
        testcase_3_2(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }

}




#include "testsuite_final.h"

void testsuite_4(test_context *ctx) {
swe_set_ephe_path(NULL);

    void set_sid_mode() {
      double t0 = get_d("t0",ctx),
             ayan_t0 = get_d("ayan_t0",ctx);
      int    sid_mode = get_i("sid_mode",ctx);
      swe_set_sid_mode(sid_mode,t0,ayan_t0);
    }

    void testcase_4_1(test_context *ctx) {

    // Set specified sidereal mode
       set_sid_mode( );

    // swe_calc
      double jd = get_d("jd",ctx);
      int iflag = get_i("iflag",ctx);
      int ipl = get_i("ipl",ctx);
      double xx[6];
      char serr[255];
      //fprintf(stderr, "jd=%.17f, ipl=%d, iflag=%d, \n", jd, ipl, iflag);
      int rc = swe_calc(jd,ipl,iflag,xx,serr);
      //fprintf(stderr, "x0=%.17f, x3=%.17f\n", xx[0], xx[3]);
      check_swecalc_results(rc,xx,serr,ctx);

    }

    void testcase_4_2(test_context *ctx) {

      double daya_ex_ut,daya_ex;
      char serr_ex_ut[255], serr_ex[255];
      *serr_ex_ut = *serr_ex = '\0';

      set_sid_mode( );

      double jd = get_d("jd",ctx);
      int iephe = get_i("iephe",ctx);

    // swe_get_ayanamsa_ex_ut
      int rc_aya_ex_ut = swe_get_ayanamsa_ex_ut(jd, iephe, &daya_ex_ut, serr_ex_ut);
      check_i(rc_aya_ex_ut,"rc_aya_ex_ut",ctx);
      check_d(daya_ex_ut,"daya_ex_ut",ctx);
      check_s(serr_ex_ut,"serr_ex_ut",ctx);

    // swe_get_ayanamsa_ex
      int rc_aya_ex = swe_get_ayanamsa_ex(jd, iephe, &daya_ex, serr_ex);
      check_i(rc_aya_ex,"rc_aya_ex",ctx);
      check_d(daya_ex,"daya_ex",ctx);
      check_s(serr_ex,"serr_ex",ctx);

    }


    void testcase_4_3(test_context *ctx) {

      set_sid_mode( );

      double jd = get_d("jd",ctx);

    // swe_get_ayanamsa_ut
      double daya_ut = swe_get_ayanamsa_ut(jd);
      check_d(daya_ut,"daya_ut",ctx);

    // swe_get_ayanamsa
      double daya = swe_get_ayanamsa(jd);
      check_d(daya,"daya",ctx);

    }

    void testcase_4_4(test_context *ctx) {

      // Set sidereal mode as given in fixture file
      set_sid_mode( );

      // Get ayanamsa...
      double jd = get_d("jd",ctx);
      double daya = swe_get_ayanamsa_ut(jd);

      // ... and check it
      check_d(daya,"daya",ctx);


    }




  open_testsuite(4,"Some computations in sidereal mode",ctx);


  if (is_testcase_selected( 1, ctx)) { 
    open_testcase(1,"swe_set_aynamsa( ) - Calc planet using a specific Ayanamsa",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        
        testcase_4_1(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 2, ctx)) { 
    open_testcase(2,"swe_get_ayanamsa_ex and swe_get_ayanamsa_ex_ut",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        
        testcase_4_2(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 3, ctx)) { 
    open_testcase(3,"swe_get_aynamsa and swe_get_ayanamsa_ut",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        
        testcase_4_3(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 4, ctx)) { 
    open_testcase(4,"Initialization problem with swe_set_sid_mode",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        
        testcase_4_4(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }

}
#include "testsuite_final.h"


void testsuite_5(test_context *ctx) {

  char serr[255];
  int rc;
swe_set_ephe_path(NULL);

  void testcase_5_1(test_context *ctx) {
    double jd = swe_julday(
        get_i("year",ctx),
        get_i("month",ctx),
        get_i("day",ctx),
        get_d("hour",ctx),
        get_i("gregflag",ctx));
    check_d(jd,"jd",ctx);
  }

  void testcase_5_2(test_context *ctx) {
    int year,month,day;
    double ut;
    swe_revjul (get_d("jd",ctx), get_i("gregflag",ctx),
           &year, &month, &day, &ut);
    check_i(year,"year",ctx);
    check_i(month,"month",ctx);
    check_i(day,"day",ctx);
    check_d(ut,"ut",ctx);
  }

  void testcase_5_3(test_context *ctx) {
    double deltat;
    int iephe = get_i("iephe",ctx);
    double jd = get_d("jd",ctx);
    if (iephe > 0) {
//fprintf(stderr, "jd=%.11f, ie=%d\n", jd, iephe);
      deltat = swe_deltat_ex(jd,iephe,serr);
//fprintf(stderr, "dt=%.11f  s=%s\n", deltat, serr);
      check_s(serr,"serr",ctx);
    }
    else {
      deltat = swe_deltat(jd);
    }
    check_d(deltat,"deltat",ctx);
  }

  void testcase_5_4(test_context *ctx) {
    double E;
    int rc = swe_time_equ(get_d("jd",ctx), &E, serr);
    check_d(E,"E",ctx);
    check_i(rc,"rc",ctx);
  }

  void testcase_5_5(test_context *ctx) {
    double tjd_lmt, tjd_lat, geolon;
    tjd_lmt = get_d("tjd_lmt",ctx);
    geolon = get_d("geolon",ctx);
    rc = swe_lmt_to_lat(tjd_lmt,geolon,&tjd_lat,serr);
    check_i(rc,"rc",ctx);
    check_s(serr,"serr",ctx);
    check_d(tjd_lat,"tjd_lat",ctx);
  }

  void testcase_5_6(test_context *ctx) {
    double tjd_lmt, tjd_lat, geolon;
    tjd_lat = get_d("tjd_lat",ctx);
    geolon = get_d("geolon",ctx);
    rc = swe_lat_to_lmt(tjd_lat,geolon,&tjd_lmt,serr);
    check_i(rc,"rc",ctx);
    check_s(serr,"serr",ctx);
    check_d(tjd_lmt,"tjd_lmt",ctx);
  }




void setup_5() {
    *serr = '\0';
    rc = 0;
  }

  
  open_testsuite(5,"Date and time functions",ctx);


  if (is_testcase_selected( 1, ctx)) { 
    open_testcase(1,"Julian Date from Calendar date",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_5();
        testcase_5_1(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 2, ctx)) { 
    open_testcase(2,"Calendar date from Julian Date",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_5();
        testcase_5_2(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 3, ctx)) { 
    open_testcase(3,"Delta T",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_5();
        testcase_5_3(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 4, ctx)) { 
    open_testcase(4,"Equation of time",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_5();
        testcase_5_4(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 5, ctx)) { 
    open_testcase(5,"Local Mean Time to Local Apparent Time",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_5();
        testcase_5_5(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 6, ctx)) { 
    open_testcase(6,"Local Apparent Time to Local Mean Time",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_5();
        testcase_5_6(ctx);
        
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }

}
#include "testsuite_final.h"


void testsuite_6(test_context *ctx) {

double cusps[37],ascmc[10], jd_ut, geolat, geolon, armc,xx[6];
double cusp_speed[37], ascmc_speed[10];
char serr[255], *sp;
int rc, ihsy;
swe_close();
swe_set_ephe_path(NULL);

void testcase_6_1(test_context *ctx) {
  rc = swe_houses(jd_ut, geolat, geolon, ihsy, cusps, ascmc);
  check_swehouses_results(rc,jd_ut,ihsy,cusps,ascmc,serr,ctx);
  }
void testcase_6_2(test_context *ctx) {
  rc = swe_houses_ex(jd_ut, get_i("iflag",ctx), geolat, geolon, ihsy, cusps, ascmc);
  check_swehouses_results(rc,jd_ut,ihsy,cusps,ascmc,serr,ctx);
  }
void testcase_6_3(test_context *ctx) {
  swe_set_sid_mode(get_i("isid",ctx), 0, 0);
  rc = swe_houses_ex(jd_ut, get_i("iflag",ctx), geolat, geolon, ihsy, cusps, ascmc);
//if (ihsy == 'P') fprintf(stderr, " ==> %f\n", cusps[1]);
  check_swehouses_results(rc,jd_ut,ihsy,cusps,ascmc,serr,ctx);
  }
void testcase_6_4(test_context *ctx) {
  double eps;
  rc = swe_calc(jd_ut, SE_ECL_NUT, 0, xx, serr);	// 
  eps = xx[0];
  armc = swe_degnorm(swe_sidtime(jd_ut) + geolon);
  rc = swe_houses_armc(armc, geolat, eps, ihsy, cusps, ascmc);
  check_swehouses_armc_results(rc,armc,ihsy,cusps,ascmc,serr,ctx);
  }
void testcase_6_5(test_context *ctx) {
  sp = swe_house_name(ihsy);
  check_s(sp,"sp",ctx);
 }
void testcase_6_6(test_context *ctx) {
  double hp;
  double eps;
  // jd_ut loops through a day, 
  rc = swe_calc(jd_ut, SE_ECL_NUT, 0, xx, serr);	// obliquity eps
  eps = xx[0];
  armc = swe_degnorm(swe_sidtime(jd_ut) * 15 + geolon);
  rc = swe_calc(jd_ut, SE_SUN, 0, xx, serr);	// sun pos xx
  hp = swe_house_pos(armc, geolat, eps, ihsy, xx, serr);
  check_d(armc,"armc",ctx);	// to show
  check_d(xx[0],"xx[0]",ctx);	// to show
  check_d(hp,"hp",ctx);
  }
void testcase_6_7(test_context *ctx) {
  // jd_ut loops through a day, 
  int imeth = get_i("imeth",ctx);
  double gp;
  double geopos[3] = {geolon, geolat, 100};
  rc = swe_gauquelin_sector(jd_ut, SE_SUN, NULL, 0, imeth, geopos, 0.0, 20.0, &gp, serr);	
  check_d(jd_ut,"jd_ut",ctx);	// to show
  check_d(gp,"gp",ctx);
  check_i(rc,"rc",ctx);
  check_s(serr,"serr",ctx);
  }
void testcase_6_8(test_context *ctx) {
  rc = swe_houses_ex2(jd_ut, get_i("iflag",ctx), geolat, geolon, ihsy, cusps, ascmc, cusp_speed, ascmc_speed, serr);
//if (ihsy == 'P') fprintf(stderr, " ==> %f\n", cusps[1]);
  check_swehouses_ex2_results(rc,jd_ut,ihsy,cusps,ascmc,cusp_speed,ascmc_speed,serr,ctx);
  }
void testcase_6_9(test_context *ctx) {
  double eps;
  rc = swe_calc(jd_ut, SE_ECL_NUT, 0, xx, serr);	// 
  eps = xx[0];
  armc = swe_degnorm(swe_sidtime(jd_ut) + geolon);
  rc = swe_houses_armc_ex2(armc, geolat, eps, ihsy, cusps, ascmc,cusp_speed,ascmc_speed,serr);
  check_swehouses_armc_ex2_results(rc,armc,ihsy,cusps,ascmc,cusp_speed,ascmc_speed,serr,ctx);
  }

void setup_6() {
  *serr = '\0';
  jd_ut = get_d("jd",ctx) + get_d("ut",ctx) / 24.0;
  ihsy = get_i("ihsy",ctx);
  geolat = get_d("geolat",ctx);
  geolon = get_d("geolon",ctx);
  }

void teardown_6() {
  if (get_i("initialize",ctx)) swe_close( );
  }


  open_testsuite(6,"Houses functions",ctx);


  if (is_testcase_selected( 1, ctx)) { 
    open_testcase(1,"swe_houses( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_6();
        testcase_6_1(ctx);
        teardown_6();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 2, ctx)) { 
    open_testcase(2,"swe_houses_ex( ) - degrees and radians",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_6();
        testcase_6_2(ctx);
        teardown_6();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 3, ctx)) { 
    open_testcase(3,"swe_houses_ex( ) - sidereal ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_6();
        testcase_6_3(ctx);
        teardown_6();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 4, ctx)) { 
    open_testcase(4,"swe_houses_armc( ) - Koch and Placidus",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_6();
        testcase_6_4(ctx);
        teardown_6();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 5, ctx)) { 
    open_testcase(5,"swe_house_name( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_6();
        testcase_6_5(ctx);
        teardown_6();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 6, ctx)) { 
    open_testcase(6,"swe_house_pos( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_6();
        testcase_6_6(ctx);
        teardown_6();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 7, ctx)) { 
    open_testcase(7,"swe_gauquelin_sector( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_6();
        testcase_6_7(ctx);
        teardown_6();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 8, ctx)) { 
    open_testcase(8,"swe_houses_ex2( ) - with speeds of cusps ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_6();
        testcase_6_8(ctx);
        teardown_6();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 9, ctx)) { 
    open_testcase(9,"swe_houses_armc_ex2( ) - Koch and Placidus with cusp speeds",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_6();
        testcase_6_9(ctx);
        teardown_6();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }

}
#include "testsuite_final.h"


void testsuite_7(test_context *ctx) {

double jd, xxnasc[6],xxndsc[6],xxperi[6],xxaphe[6], xxdret[20];
int iflag, iephe; // Keep ephemeris selector separate from other flags
char serr[255];
int rc;
swe_set_ephe_path(NULL);

void testcase_7_1(test_context *ctx) {
  jd = get_d("jd",ctx);
  rc = swe_nod_aps(jd, get_i("ipl",ctx), iflag|iephe,  get_i("method",ctx), xxnasc, xxndsc, xxperi, xxaphe, serr);
  check_i(rc,"rc",ctx);
  check_s(serr,"serr",ctx);
  check_dd(xxnasc,6,"xxnasc",ctx);
  check_dd(xxndsc,6,"xxndsc",ctx);
  check_dd(xxperi,6,"xxperi",ctx);
  check_dd(xxaphe,6,"xxaphe",ctx);
  }

void testcase_7_2(test_context *ctx) {
  jd = get_d("jd",ctx);
  rc = swe_nod_aps_ut(jd, get_i("ipl",ctx), iflag|iephe,  get_i("method",ctx), xxnasc, xxndsc, xxperi, xxaphe, serr);
  check_i(rc,"rc",ctx);
  check_s(serr,"serr",ctx);
  check_dd(xxnasc,6,"xxnasc",ctx);
  check_dd(xxndsc,6,"xxndsc",ctx);
  check_dd(xxperi,6,"xxperi",ctx);
  check_dd(xxaphe,6,"xxaphe",ctx);
  }

void testcase_7_3(test_context *ctx) {
  jd = get_d("jd",ctx);
  rc = swe_get_orbital_elements(jd, get_i("ipl",ctx), iflag|iephe,  xxdret, serr);
  check_i(rc,"rc",ctx);
  check_s(serr,"serr",ctx);
  check_dd(xxdret,17,"xxdret",ctx);
  }

void testcase_7_4(test_context *ctx) {
  double dmax, dmin, dtrue;
  jd = get_d("jd",ctx);
  rc = swe_orbit_max_min_true_distance(jd, get_i("ipl",ctx), iflag|iephe,  &dmax, &dmin, &dtrue, serr);
  check_i(rc,"rc",ctx);
  check_s(serr,"serr",ctx);
  check_d(dmax,"dmax",ctx);
  check_d(dmin,"dmin",ctx);
  check_d(dtrue,"dtrue",ctx);
  }

void setup_7() {
  iflag = get_i("iflag",ctx);
  iephe = get_i("iephe",ctx);
  *serr = '\0';
  }

void teardown_7() {
  if (get_i("initialize",ctx)) swe_close( );
  }


  open_testsuite(7,"Apsides and orbital elements functions",ctx);


  if (is_testcase_selected( 1, ctx)) { 
    open_testcase(1,"swe_nod_aps( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_7();
        testcase_7_1(ctx);
        teardown_7();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 2, ctx)) { 
    open_testcase(2,"swe_nod_aps_ut( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_7();
        testcase_7_2(ctx);
        teardown_7();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 3, ctx)) { 
    open_testcase(3,"swe_get_orbital_elements( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_7();
        testcase_7_3(ctx);
        teardown_7();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 4, ctx)) { 
    open_testcase(4,"swe_orbit_max_min_true_distance( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_7();
        testcase_7_4(ctx);
        teardown_7();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }

}
#include "testsuite_final.h"


void testsuite_8(test_context *ctx) {

double jd, xxtret[10], xxgeopos[3], xxattr[20];
double geolat, geolon, altitude;
char serr[255], star[100];;
int rc, ipl;
swe_set_ephe_path(NULL);

void testcase_8_1(test_context *ctx) {
  jd = get_d("jd",ctx);
  rc = swe_sol_eclipse_when_glob(jd, get_i("iephe",ctx), get_i("ifltype",ctx),
       xxtret, get_i("backward",ctx), serr);
  check_i(rc,"rc",ctx);
  check_dd(xxtret,10,"xxtret",ctx);
  }

void testcase_8_2(test_context *ctx) {
  jd = get_d("jd",ctx);
  rc = swe_sol_eclipse_where(jd, get_i("iephe",ctx), xxgeopos, xxattr, serr);
  check_i(rc,"rc",ctx);
  check_dd(xxgeopos,2,"xxgeopos",ctx);
  check_dd(xxattr,8,"xxattr",ctx);
  }

void testcase_8_3(test_context *ctx) {
  jd = get_d("jd",ctx);
  double geopos[3] = {geolon, geolat, altitude};
  rc = swe_sol_eclipse_when_loc(jd, get_i("iephe",ctx), geopos, xxtret, xxattr, get_i("backward",ctx), serr);
  check_i(rc,"rc",ctx);
  check_dd(xxtret,7,"xxtret",ctx);
  check_dd(xxattr,11,"xxattr",ctx);
  }

void testcase_8_4(test_context *ctx) {
  jd = get_d("jd",ctx);
  double geopos[3] = {geolon, geolat, altitude};
  rc = swe_sol_eclipse_how(jd, get_i("iephe",ctx), geopos, xxattr, serr);
  check_i(rc,"rc",ctx);
  check_dd(xxattr,11,"xxattr",ctx);
  }

void testcase_8_5(test_context *ctx) {
  jd = get_d("jd",ctx);
  ipl = get_i("ipl",ctx);
  *star = '\0';
  if (ipl == -1)
    strcpy(star,get_s("star",ctx));
  rc = swe_lun_occult_when_glob(jd, ipl, star, get_i("iephe",ctx), get_i("ifltype",ctx),
       xxtret, get_i("backward",ctx), serr);
  check_i(rc,"rc",ctx);
  check_dd(xxtret,10,"xxtret",ctx);
  }

void testcase_8_6(test_context *ctx) {
  jd = get_d("jd",ctx);
  ipl = get_i("ipl",ctx);
  *star = '\0';
  if (ipl == -1)
    strcpy(star,get_s("star",ctx));
  rc = swe_lun_occult_where(jd, ipl, star, get_i("iephe",ctx), xxgeopos, xxattr, serr);
  check_dd(xxgeopos,2,"xxgeopos",ctx);
  check_dd(xxattr,8,"xxattr",ctx);
  }

void testcase_8_7(test_context *ctx) {
  double geopos[3] = {geolon, geolat, altitude};
  jd = get_d("jd",ctx);
  ipl = get_i("ipl",ctx);
  *star = '\0';
  if (ipl == -1)
    strcpy(star,get_s("star",ctx));
  rc = swe_lun_occult_when_loc(jd, ipl, star, get_i("iephe",ctx), geopos,
       xxtret, xxattr, get_i("backward",ctx), serr);
  check_i(rc,"rc",ctx);
  check_dd(xxtret,7,"xxtret",ctx);
  check_dd(xxattr,11,"xxattr",ctx);
  }

void testcase_8_8(test_context *ctx) {
  jd = get_d("jd",ctx);
  rc = swe_lun_eclipse_when(jd, get_i("iephe",ctx), get_i("ifltype",ctx),
       xxtret, get_i("backward",ctx), serr);
  check_i(rc,"rc",ctx);
  check_dd(xxtret,10,"xxtret",ctx);
  }

void testcase_8_9(test_context *ctx) {
  jd = get_d("jd",ctx);
  double geopos[3] = {geolon, geolat, altitude};
  rc = swe_sol_eclipse_when_loc(jd, get_i("iephe",ctx), geopos, xxtret, xxattr, get_i("backward",ctx), serr);
  check_i(rc,"rc",ctx);
  check_dd(xxtret,10,"xxtret",ctx);
  check_dd(xxattr,11,"xxattr",ctx);
  }

void testcase_8_10(test_context *ctx) {
  jd = get_d("jd",ctx);
  double geopos[3] = {geolon, geolat, altitude};
  rc = swe_lun_eclipse_how(jd, get_i("iephe",ctx), geopos, xxattr, serr);
  check_i(rc,"rc",ctx);
  check_dd(xxattr,11,"xxattr",ctx);
  }

void setup_8() {
  *serr = '\0';
  geolat = get_d("geolat",ctx);
  geolon = get_d("geolon",ctx);
  altitude = get_d("altitude",ctx);
  }

void teardown_8() {
  if (get_i("initialize",ctx)) swe_close( );
  }


  open_testsuite(8,"Eclipses",ctx);


  if (is_testcase_selected( 1, ctx)) { 
    open_testcase(1,"swe_sol_eclipse_when_glob( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_8();
        testcase_8_1(ctx);
        teardown_8();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 2, ctx)) { 
    open_testcase(2,"swe_sol_eclipse_where( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_8();
        testcase_8_2(ctx);
        teardown_8();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 3, ctx)) { 
    open_testcase(3,"swe_sol_eclipse_when_loc( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_8();
        testcase_8_3(ctx);
        teardown_8();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 4, ctx)) { 
    open_testcase(4,"swe_sol_eclipse_how( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_8();
        testcase_8_4(ctx);
        teardown_8();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 5, ctx)) { 
    open_testcase(5,"swe_lun_occult_when_glob( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_8();
        testcase_8_5(ctx);
        teardown_8();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 6, ctx)) { 
    open_testcase(6,"swe_lun_occult_where( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_8();
        testcase_8_6(ctx);
        teardown_8();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 7, ctx)) { 
    open_testcase(7,"swe_lun_occult_when_loc( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_8();
        testcase_8_7(ctx);
        teardown_8();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 8, ctx)) { 
    open_testcase(8,"swe_lun_eclipse_when( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_8();
        testcase_8_8(ctx);
        teardown_8();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 9, ctx)) { 
    open_testcase(9,"swe_lun_eclipse_when_loc( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_8();
        testcase_8_9(ctx);
        teardown_8();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 10, ctx)) { 
    open_testcase(10,"swe_lun_eclipse_how( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_8();
        testcase_8_10(ctx);
        teardown_8();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }

}
#include "testsuite_final.h"


void testsuite_9(test_context *ctx) {

double jd, xxtret[10];
double geolat, geolon, altitude, tret;
double atpress, attemp, athumid, atktot, obsage, obsSN;
char serr[255], star[100], object[100];
int rc, ipl;
swe_set_ephe_path(NULL);

void testcase_9_1(test_context *ctx) {
  double geopos[3] = {geolon, geolat, altitude};
  int rsmi = get_i("ifltype",ctx) | get_i("method",ctx);
  jd = get_d("jd",ctx);
  ipl = get_i("ipl",ctx);
  *star = '\0';
  if (ipl == -1)
    strcpy(star,get_s("star",ctx));
  rc = swe_rise_trans(jd, get_i("ipl",ctx), star, get_i("iephe",ctx), rsmi,
       geopos, get_d("atpress",ctx), get_d("attemp",ctx), &tret, serr);
  check_i(rc,"rc",ctx);
  check_d(tret,"tret",ctx);
  }

void testcase_9_2(test_context *ctx) {
  double geopos[3] = {geolon, geolat, altitude};
  int rsmi = get_i("ifltype",ctx) | get_i("method",ctx);
  jd = get_d("jd",ctx);
  ipl = get_i("ipl",ctx);
  *star = '\0';
  if (ipl == -1)
    strcpy(star,get_s("star",ctx));
  rc = swe_rise_trans_true_hor(jd, get_i("ipl",ctx), star, get_i("iephe",ctx), rsmi,
       geopos, get_d("atpress",ctx), get_d("attemp",ctx), get_d("horhgt",ctx), &tret, serr);
  check_i(rc,"rc",ctx);
  check_d(tret,"tret",ctx);
  }

void testcase_9_3(test_context *ctx) {
  double geopos[3] = {geolon, geolat, altitude};
  double datm[4] = {atpress, attemp, athumid, atktot};
  double dobs[5] = {obsage, obsSN,};
  jd = get_d("jd",ctx);
  rc = swe_heliacal_ut(jd, geopos, datm, dobs, strcpy(object,get_s("object",ctx)), get_i("evtype",ctx), get_i("helflag",ctx), xxtret, serr);
  check_i(rc,"rc",ctx);
  check_dd(xxtret,3,"xxtret",ctx);
  }

void testcase_9_4(test_context *ctx) {
  double geopos[3] = {geolon, geolat, altitude};
  double datm[4] = {atpress, attemp, athumid, atktot};
  double dobs[5] = {obsage, obsSN,};
  jd = get_d("jd",ctx);
  rc = swe_heliacal_pheno_ut(jd, geopos, datm, dobs, strcpy(object,get_s("object",ctx)), get_i("evtype",ctx), get_i("helflag",ctx), xxtret, serr);
  check_i(rc,"rc",ctx);
  check_dd(xxtret,3,"xxtret",ctx);
  }

void testcase_9_5(test_context *ctx) {
  double geopos[3] = {geolon, geolat, altitude};
  double datm[4] = {atpress, attemp, athumid, atktot};
  double dobs[5] = {obsage, obsSN,};
  jd = get_d("jd",ctx);
  rc = swe_vis_limit_mag(jd, geopos, datm, dobs, strcpy(object,get_s("object",ctx)), get_i("helflag",ctx), xxtret, serr);
  check_i(rc,"rc",ctx);
  check_dd(xxtret,3,"xxtret",ctx);
  }

void setup_9() {
  *serr = '\0';
  geolat = get_d("geolat",ctx);
  geolon = get_d("geolon",ctx);
  altitude = get_d("altitude",ctx);
  atpress = get_d("atpress",ctx);
  attemp = get_d("attemp",ctx);
  athumid = get_d("athumid",ctx);
  atktot = get_d("atktot",ctx);
  obsage = get_d("obsage",ctx);
  obsSN = get_d("obsSN",ctx);
  }

void teardown_9() {
  if (get_i("initialize",ctx)) swe_close( );
  }


  open_testsuite(9,"risings, heliacal risings",ctx);


  if (is_testcase_selected( 1, ctx)) { 
    open_testcase(1,"swe_rise_trans( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_9();
        testcase_9_1(ctx);
        teardown_9();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 2, ctx)) { 
    open_testcase(2,"swe_rise_trans_true_hor( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_9();
        testcase_9_2(ctx);
        teardown_9();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 3, ctx)) { 
    open_testcase(3,"swe_heliacal_ut( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_9();
        testcase_9_3(ctx);
        teardown_9();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 4, ctx)) { 
    open_testcase(4,"swe_heliacal_pheno_ut()( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_9();
        testcase_9_4(ctx);
        teardown_9();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 5, ctx)) { 
    open_testcase(5,"swe_vis_limit_mag()( ) ",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_9();
        testcase_9_5(ctx);
        teardown_9();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }

}
#include "testsuite_final.h"



void testsuite_10(test_context *ctx) {

double xx[6],jd;
int iflag, iephe; // Keep ephemeris selector separate from other flags
char serr[255];

swe_set_ephe_path(NULL);
swe_set_jpl_file("de431.eph");

void testcase_10_1(test_context *ctx) {
  iflag = iephe;
  double  xcross = get_d("xcross",ctx);
  double jx = swe_solcross(xcross, jd,  iflag, serr);
  int rc = swe_calc(jx, SE_SUN, iflag, xx, serr);
  check_i(rc,"rc",ctx);
  check_d(jx,"jx",ctx);
  check_equals_d(xcross,xx[0],"xcross",ctx);
  check_s(serr,"serr",ctx);
  }

void testcase_10_2(test_context *ctx) {
  iflag = iephe;
  double  xcross = get_d("xcross",ctx);
  double jx = swe_solcross_ut(xcross, jd,  iflag, serr);
  int rc = swe_calc_ut(jx, SE_SUN, iflag, xx, serr);
  check_i(rc,"rc",ctx);
  check_d(jx,"jx",ctx);
  check_equals_d(xcross,xx[0],"xcross",ctx);
  check_s(serr,"serr",ctx);
  }

void testcase_10_3(test_context *ctx) {
  iflag = iephe;
  double  xcross = get_d("xcross",ctx);
  double jx = swe_mooncross(xcross, jd,  iflag, serr);
  int rc = swe_calc(jx, SE_MOON, iflag, xx, serr);
  check_i(rc,"rc",ctx);
  check_d(jx,"jx",ctx);
  check_equals_d(xcross,xx[0],"xcross",ctx);
  check_s(serr,"serr",ctx);
  }

void testcase_10_4(test_context *ctx) {
  iflag = iephe;
  double  xcross = get_d("xcross",ctx);
  double jx = swe_mooncross_ut(xcross, jd,  iflag, serr);
  int rc = swe_calc_ut(jx, SE_MOON, iflag, xx, serr);
  check_i(rc,"rc",ctx);
  check_d(jx,"jx",ctx);
  check_equals_d(xcross,xx[0],"xcross",ctx);
  check_s(serr,"serr",ctx);
  }

void testcase_10_5(test_context *ctx) {
  iflag = iephe;
  double xlon, xlat;
  double jx = swe_mooncross_node(jd,  iflag, &xlon, &xlat, serr);
  int rc = swe_calc(jx, SE_MOON, iflag, xx, serr);
  check_i(rc,"rc",ctx);
  check_d(jx,"jx",ctx);
  check_d(xlon,"xlon",ctx);
  check_d(xlat,"xlat",ctx);
  check_equals_d(xx[1],0,"xx[1]",ctx);	// latitude must  be zero
  check_s(serr,"serr",ctx);
  }

void testcase_10_6(test_context *ctx) {
  iflag = iephe;
  double xlon, xlat;
  double jx = swe_mooncross_node_ut(jd,  iflag, &xlon, &xlat, serr);
  int rc = swe_calc_ut(jx, SE_MOON, iflag, xx, serr);
  check_i(rc,"rc",ctx);
  check_d(jx,"jx",ctx);
  check_d(xlon,"xlon",ctx);
  check_d(xlat,"xlat",ctx);
  check_equals_d(xx[1],0,"xx[1]",ctx);	// latitude must  be zero
  check_s(serr,"serr",ctx);
  }

void testcase_10_7(test_context *ctx) {
  iflag = iephe;
  double jx;
  double  xcross = get_d("xcross",ctx);
  int ipl = get_i("ipl",ctx);
  int dir = get_i("dir",ctx);
  int rc = swe_helio_cross(ipl, xcross, jd,  iflag, dir, &jx, serr);
  check_d(jx,"jx",ctx);
  check_i(rc,"rc",ctx);
  check_s(serr,"serr",ctx);
  }

void testcase_10_8(test_context *ctx) {
  iflag = iephe;
  double jx;
  double  xcross = get_d("xcross",ctx);
  int ipl = get_i("ipl",ctx);
  int dir = get_i("dir",ctx);
  int rc = swe_helio_cross_ut(ipl, xcross, jd,  iflag, dir, &jx, serr);
  check_d(jx,"jx",ctx);
  check_i(rc,"rc",ctx);
  check_s(serr,"serr",ctx);
  }


void setup_10() {
  iephe = get_i("iephe",ctx);
  jd = get_d("jd",ctx);
  *serr = '\0';
  }

void teardown_10() {
  if (get_i("initialize",ctx)) swe_close( );
  }

  open_testsuite(10,"Various swe_*cross calls ",ctx);


  if (is_testcase_selected( 1, ctx)) { 
    open_testcase(1,"swe_solcross( ) - sun crossing a position",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_10();
        testcase_10_1(ctx);
        teardown_10();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 2, ctx)) { 
    open_testcase(2,"swe_solcross_ut( ) - sun crossing a position",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_10();
        testcase_10_2(ctx);
        teardown_10();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 3, ctx)) { 
    open_testcase(3,"swe_mooncross( ) - moon crossing a position",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_10();
        testcase_10_3(ctx);
        teardown_10();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 4, ctx)) { 
    open_testcase(4,"swe_mooncross_ut( ) - moon crossing a position",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_10();
        testcase_10_4(ctx);
        teardown_10();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 5, ctx)) { 
    open_testcase(5,"swe_mooncross_node( ) - moon crossing node position",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_10();
        testcase_10_5(ctx);
        teardown_10();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 6, ctx)) { 
    open_testcase(6,"swe_mooncross_node_ut( ) - moon crossing node position",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_10();
        testcase_10_6(ctx);
        teardown_10();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 7, ctx)) { 
    open_testcase(7,"swe_helio_cross( ) - heliocentric crossing planet over a position",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_10();
        testcase_10_7(ctx);
        teardown_10();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }


  if (is_testcase_selected( 8, ctx)) { 
    open_testcase(8,"swe_helio_cross_ut( ) - heliocentric crossing planet over a position",ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        setup_10();
        testcase_10_8(ctx);
        teardown_10();
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }

}

void run_tests(test_context *ctx) {
  if (is_suite_selected(1,ctx)) {
    strcpy(ctx->current.file,"suite_01_calc.c");
    testsuite_1(ctx);
    close_testsuite(ctx);
  }  
  if (is_suite_selected(2,ctx)) {
    strcpy(ctx->current.file,"suite_02_fixstar.c");
    testsuite_2(ctx);
    close_testsuite(ctx);
  }  
  if (is_suite_selected(3,ctx)) {
    strcpy(ctx->current.file,"suite_03_misc.c");
    testsuite_3(ctx);
    close_testsuite(ctx);
  }  
  if (is_suite_selected(4,ctx)) {
    strcpy(ctx->current.file,"suite_04_ayanamsa.c");
    testsuite_4(ctx);
    close_testsuite(ctx);
  }  
  if (is_suite_selected(5,ctx)) {
    strcpy(ctx->current.file,"suite_05_date_time.c");
    testsuite_5(ctx);
    close_testsuite(ctx);
  }  
  if (is_suite_selected(6,ctx)) {
    strcpy(ctx->current.file,"suite_06_houses.c");
    testsuite_6(ctx);
    close_testsuite(ctx);
  }  
  if (is_suite_selected(7,ctx)) {
    strcpy(ctx->current.file,"suite_07_apsides.c");
    testsuite_7(ctx);
    close_testsuite(ctx);
  }  
  if (is_suite_selected(8,ctx)) {
    strcpy(ctx->current.file,"suite_08_eclipses.c");
    testsuite_8(ctx);
    close_testsuite(ctx);
  }  
  if (is_suite_selected(9,ctx)) {
    strcpy(ctx->current.file,"suite_09_rise.c");
    testsuite_9(ctx);
    close_testsuite(ctx);
  }  
  if (is_suite_selected(10,ctx)) {
    strcpy(ctx->current.file,"suite_10_solcross.c");
    testsuite_10(ctx);
    close_testsuite(ctx);
  }  
}
