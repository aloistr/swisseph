#include "testsuite_facade.h"
TESTSUITE(4, "Some computations in sidereal mode")
swe_set_ephe_path(NULL);

    void set_sid_mode() {
      double t0 = GET_D(t0),
             ayan_t0 = GET_D(ayan_t0);
      int    sid_mode = GET_I(sid_mode);
      swe_set_sid_mode(sid_mode,t0,ayan_t0);
    }

    TESTCASE(1, "swe_set_aynamsa( ) - Calc planet using a specific Ayanamsa") {

    // Set specified sidereal mode
       set_sid_mode( );

    // swe_calc
      double jd = GET_D(jd);
      int iflag = GET_I(iflag);
      int ipl = GET_I(ipl);
      double xx[6];
      char serr[255];
      //fprintf(stderr, "jd=%.17f, ipl=%d, iflag=%d, \n", jd, ipl, iflag);
      int rc = swe_calc(jd,ipl,iflag,xx,serr);
      //fprintf(stderr, "x0=%.17f, x3=%.17f\n", xx[0], xx[3]);
      check_swecalc_results(rc,xx,serr,ctx);

    }

    TESTCASE(2, "swe_get_ayanamsa_ex and swe_get_ayanamsa_ex_ut") {

      double daya_ex_ut,daya_ex;
      char serr_ex_ut[255], serr_ex[255];
      *serr_ex_ut = *serr_ex = '\0';

      set_sid_mode( );

      double jd = GET_D(jd);
      int iephe = GET_I(iephe);

    // swe_get_ayanamsa_ex_ut
      int rc_aya_ex_ut = swe_get_ayanamsa_ex_ut(jd, iephe, &daya_ex_ut, serr_ex_ut);
      CHECK_I(rc_aya_ex_ut);
      CHECK_D(daya_ex_ut);
      CHECK_S(serr_ex_ut);

    // swe_get_ayanamsa_ex
      int rc_aya_ex = swe_get_ayanamsa_ex(jd, iephe, &daya_ex, serr_ex);
      CHECK_I(rc_aya_ex);
      CHECK_D(daya_ex);
      CHECK_S(serr_ex);

    }


    TESTCASE(3, "swe_get_aynamsa and swe_get_ayanamsa_ut") {

      set_sid_mode( );

      double jd = GET_D(jd);

    // swe_get_ayanamsa_ut
      double daya_ut = swe_get_ayanamsa_ut(jd);
      CHECK_D(daya_ut);

    // swe_get_ayanamsa
      double daya = swe_get_ayanamsa(jd);
      CHECK_D(daya);

    }

    TESTCASE(4, "Initialization problem with swe_set_sid_mode") {

      // Set sidereal mode as given in fixture file
      set_sid_mode( );

      // Get ayanamsa...
      double jd = GET_D(jd);
      double daya = swe_get_ayanamsa_ut(jd);

      // ... and check it
      CHECK_D(daya);


    }



END_TESTSUITE
