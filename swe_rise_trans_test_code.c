

/* Simple fast algorithm for risings and settings of 
 * - planets Sun, Moon, Mercury - Pluto + Lunar Nodes and Fixed stars
 * Does not work well for geographic latitudes
 * > 65 N/S for the Sun
 * > 60 N/S for the Moon and the planets
 * Beyond these limits, some risings or settings may be missed.
 */
static int32 rise_set_fast(
               double tjd_ut, int32 ipl, char *starname,
	       int32 epheflag, int32 rsmi,
               double *dgeo, 
	       double atpress, double attemp,
               double *tret,
               char *serr)
{
  int i;
  double xx[6], xaz[6], xaz2[6];
  double tjd_et, tjd_ut0 = tjd_ut;
  double deltat;
  double dd, dt, refr;
  int32 iflag = epheflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH); 
  int32 iflag0 = epheflag | (SEFLG_NONUT | SEFLG_EQUATORIAL | SEFLG_TRUEPOS);
  int32 iflagtopo = iflag | SEFLG_TOPOCTR | SEFLG_EQUATORIAL;
  AS_BOOL is_fixstar = FALSE;
  double sda, armc, md, dmd, mdrise, rdi, tr, dalt;
  double decl, declup, decllow, sdaup, sdalow;
  int32 facrise = 1;
  AS_BOOL is_second_run = FALSE;
  AS_BOOL check_transits = FALSE; // i.e. meridian transits
  int nloop = 2;
  *tret = 0;
  if (ipl == SE_MOON)
    nloop = 3; 
  if (rsmi & SE_CALC_SET)
    facrise = -1;
  if (starname != NULL && *starname != '\0') {
    is_fixstar = TRUE;
    ipl = -1;
  }
  tjd_ut -= 0.02;
run_rise_again:
  deltat = swe_deltat_ex(tjd_ut, iflag, serr);
  tjd_et = tjd_ut + deltat;
  // calculate rough declination
  if (is_fixstar) {
    if (swe_fixstar(starname, tjd_et, iflag0, xx, serr) == ERR) 
      return ERR;
  } else {
    if (swe_calc(tjd_et, ipl, iflag0, xx, serr) == ERR) 
      return ERR;
  }
  decl = xx[1];
  /* the diurnal arc is a bit fuzzy, 
   * - because the object changes declination during the day
   * - because there is refraction of light
   * Therefore we calculate som upper and lower declination.
   */
  if (dgeo[1] >= 0) {
    declup = decl + 1;
    decllow = decl;
    if (ipl == SE_MOON) {
      declup += 5;
      decllow -= 5;
    }
  } else {
    declup = decl - 1;
    decllow = decl;
    if (ipl == SE_MOON) {
      declup -= 5;
      decllow += 5;
    }
  }
  decl = xx[1];
  // semi-diurnal arcs
  sda = -tan(dgeo[1] * DEGTORAD) * tan(decl * DEGTORAD);
  sdaup = -tan(dgeo[1] * DEGTORAD) * tan(declup * DEGTORAD);
  sdalow = -tan(dgeo[1] * DEGTORAD) * tan(decllow * DEGTORAD);
  if (sdalow <= -1) {
    if (serr)
      sprintf(serr, "rise or set not found for planet %d (1)", ipl);
    return -2; // is circumpolar
  }
  if (sdaup <= -1) 
    check_transits = TRUE;
  if (sdaup >= 1) {
    if (serr)
      sprintf(serr, "rise or set not found for planet %d (2)", ipl);
    return -2; // is circumpolar
  }
  if (sdalow >= 1)
    check_transits = TRUE;
  if (sda >= 1) {
    sda = 10;  // actually sda = 0°, but we give it a value of 10° 
               // to account for refraction. value 0 would cause
               // problems
  } else if (sda <= -1) {
    sda = 180;
  } else {
    sda = acos(sda) * RADTODEG;
  }
  // sidereal time at tjd_start
  armc = swe_degnorm(swe_sidtime(tjd_ut) * 15 + dgeo[0]); 
  // meridian distance of object
  md = swe_degnorm(xx[0] - armc);
  mdrise = swe_degnorm(sda * facrise);
  //dmd = swe_degnorm(md - mdrise - 1);
  dmd = swe_degnorm(md - mdrise);
  /* we search forward, but if we are close to a rising we may 
   * miss it. In this case we take the previous one and see
   * later if the resulting rising time is later than the input time */
/*if (0 && !is_second_run) {
    if (ipl == SE_MOON) {
      if (dmd > 350) 
	dmd -= 360;
    } else {
      if (dmd > 358) 
	dmd -= 360;
    }
  } */
  // rough subsequent rising/setting time
  tr = tjd_ut + dmd / 360;
  /* if object is sun or moon and rising of upper limb is required,
   * calculate apparent radius of disk (ignoring refraction);
   * with other objects disk diameter is ignored. */
  rdi = 0;
  /* true altitude of sun, when it appears at the horizon; 
   * refraction for a body visible at the horizon at 0m above sea,
   */
  if (atpress == 0) {
    /* estimate atmospheric pressure */
    atpress = 1013.25 * pow(1 - 0.0065 * dgeo[2] / 288, 5.255);
  } 
  swe_refrac_extended(0.000001, 0, atpress, attemp, const_lapse_rate, SE_APP_TO_TRUE, xx);
  refr = xx[1] - xx[0];
//fprintf(stderr, "refr=%f, %f, %f\n", refr, xx[0], xx[1]);
  swe_set_topo(dgeo[0], dgeo[1], dgeo[2]);
  // in doubtful cases check visibility of object at north and south meridian
  if (check_transits) {
    // north meridian
    double t, dmd2, md2, armc2, mdrise2, rdi2, dh;
    mdrise2 = facrise * 180;
    dmd2 = swe_degnorm(md - mdrise);
    dmd2 -= (180 - sda);
    t = tjd_ut + dmd2 / 360;
    for (i = 0; i < 2; i++) {
      if (swe_calc_ut(t, ipl, iflagtopo, xx, serr) == ERR)
	return ERR;
      armc2 = swe_degnorm(swe_sidtime(t) * 15 + dgeo[0]); 
      md2 = swe_degnorm(xx[0] - armc2);
      dmd2 = swe_degnorm(md2 - mdrise2);
      if (dmd2 > 180) dmd2 -= 360;
      t += dmd2 / 360;
    }
    if (swe_calc_ut(t, ipl, iflagtopo, xx, serr) == ERR)
      return ERR;
    swe_azalt(t, SE_EQU2HOR, dgeo, atpress, attemp, xx, xaz);
    rdi2 = get_sun_rad_plus_refr(ipl, xx[2], rsmi, refr); 
    dh = xaz[1] + rdi2;
    if (dh >= 0) {
      if (serr)
	sprintf(serr, "rise or set not found for planet %d (3)", ipl);
      return -2; // does not have a rising
    }
    // south meridian
    t += 0.5 * facrise;
    for (i = 0; i < 2; i++) {
      if (swe_calc_ut(t, ipl, iflagtopo, xx, serr) == ERR)
	return ERR;
      armc2 = swe_degnorm(swe_sidtime(t) * 15 + dgeo[0]); 
      md2 = swe_degnorm(xx[0] - armc2);
      if (md2 > 180) md2 -= 360;
      t += md2 / 360;
    }
    if (swe_calc_ut(t, ipl, iflagtopo, xx, serr) == ERR)
      return ERR;
    swe_azalt(t, SE_EQU2HOR, dgeo, atpress, attemp, xx, xaz);
    rdi2 = get_sun_rad_plus_refr(ipl, xx[2], rsmi, refr); 
    dh = xaz[1] + rdi2;
    if (dh <= 0) {
      if (serr) 
	sprintf(serr, "rise or set not found for planet %d (4)", ipl);
      return -2; // does not have a rising
    }
#if 0
    if (xaz[1] + rdi2 <= 0.2) {
      if (rsmi & SE_CALC_RISE) {
        sprintf(serr, "cannot find rising of planet %d near jd=%f; if you absolutely need one, try without flag SE_BIT_FAST_CALC", ipl, tr);
	return -2;
      }
      if (rsmi & SE_CALC_SET) {
        sprintf(serr, "cannot find setting of planet %d near jd=%f; if you absolutely need one, try without flag SE_BIT_FAST_CALC", ipl, tr);
	return -2;
      }
    }
#endif
  }
  if (is_fixstar) {
    if (swe_fixstar_ut(starname, tr, iflagtopo, xx, serr) == ERR)
      return ERR;
  } else {
    double dtsum = 0;
#if 0
    if (tjd_ut == 2451908.500000) {
      printf("hallo\n");
      fprintf(stderr, "rdi0=%f\n", rdi);
    }
#endif
    for (i = 0; i < nloop; i++) {
      if (swe_calc_ut(tr, ipl, iflagtopo, xx, serr) == ERR)
	return ERR;
      rdi = get_sun_rad_plus_refr(ipl, xx[2], rsmi, refr); 
      swe_azalt(tr, SE_EQU2HOR, dgeo, atpress, attemp, xx, xaz);
      swe_azalt(tr + 0.001, SE_EQU2HOR, dgeo, atpress, attemp, xx, xaz2);
      dd = (xaz2[1] - xaz[1]);
      dalt = xaz[1] + rdi;
      dt = dalt / dd / 1000.0;
      if (dt > 0.1) dt = 0.1;
      else if (dt < -0.1) dt = -0.1;
      dtsum += dt;
      if (fabs(dt) > 5.0 / 86400.0 && nloop < 20)
        nloop++;
      tr -= dt;
    }
//    if (tjd_ut == 2451780.500000)
//      fprintf(stderr, "dalt=%f\n", dalt);
    if (fabs(dalt) > 0.01) {
      if (serr)
	sprintf(serr, "rise or set not found for planet %d (5)", ipl);
      return -2;
    }
    if ((rsmi & SE_CALC_RISE) && xaz[0] < 180) {
      sprintf(serr, "cannot find rising of planet %d near jd=%f; if you absolutely need one, try without flag SE_BIT_FAST_CALC", ipl, tr);
      return -2;
    }
    if ((rsmi & SE_CALC_SET) && xaz[0] > 180) {
      sprintf(serr, "cannot find setting of planet %d near jd=%f; if you absolutely need one, try without flag SE_BIT_FAST_CALC", ipl, tr);
      return -2;
    }
    //  fprintf(stderr, "rdi=%f\n", rdi);
#if 0
    if (tjd_ut == 2451908.500000) {
      fprintf(stderr, "rdi=%f\n", rdi);
    }
    if (fabs(dtsum) > 1)
      fprintf(stderr, "dtsum=%f, nloop=%d\n", dtsum, nloop);
#endif
  }
//fprintf(stderr, "tr-tjd=%f tin=%f tout=%f\n", tr - tjd_ut, tjd_ut, tr);
  if (tr < tjd_ut0 && !is_second_run) {
    tjd_ut += 0.5; 
    is_second_run = TRUE;
    goto run_rise_again;
  } 
  *tret = tr;
  return OK;
}


static int32 rise_set_fast_obsolet(
               double tjd_ut, int32 ipl, char *starname,
	       int32 epheflag, int32 rsmi,
               double *dgeo, 
	       double atpress, double attemp,
               double *tret,
               char *serr)
{
  int i;
  double xx[6], xaz[6], xaz2[6];
  double tjd_et, tjd_ut0 = tjd_ut;
  double deltat;
  double dd, dt, refr;
  int32 iflag = epheflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH); 
  int32 iflag0 = epheflag | (SEFLG_NONUT | SEFLG_EQUATORIAL | SEFLG_TRUEPOS);
  int32 iflagtopo = iflag | SEFLG_TOPOCTR | SEFLG_EQUATORIAL;
  AS_BOOL is_fixstar = FALSE;
  double sda, armc, md, dmd, mdrise, rdi, tr, dalt;
  double decl, declup, decllow, sdaup, sdalow;
  int32 facrise = 1;
  AS_BOOL is_second_run = FALSE;
  AS_BOOL check_transits = FALSE; // i.e. meridian transits
  int nloop = 2;
  *tret = 0;
  if (ipl == SE_MOON)
    nloop = 3; 
  if (rsmi & SE_CALC_SET)
    facrise = -1;
  if (starname != NULL && *starname != '\0') {
    is_fixstar = TRUE;
    ipl = -1;
  }
  tjd_ut -= 0.02;
run_rise_again:
  deltat = swe_deltat_ex(tjd_ut, iflag, serr);
  tjd_et = tjd_ut + deltat;
  // calculate rough declination
  if (is_fixstar) {
    if (swe_fixstar(starname, tjd_et, iflag0, xx, serr) == ERR) 
      return ERR;
  } else {
    if (swe_calc(tjd_et, ipl, iflag0, xx, serr) == ERR) 
      return ERR;
  }
  decl = xx[1];
  /* the diurnal arc is a bit fuzzy, 
   * - because the object changes declination during the day
   * - because there is refraction of light
   * Therefore we calculate som upper and lower declination.
   */
  if (dgeo[1] >= 0) {
    declup = decl + 1;
    decllow = decl;
    if (ipl == SE_MOON) {
      declup += 5;
      decllow -= 5;
    }
  } else {
    declup = decl - 1;
    decllow = decl;
    if (ipl == SE_MOON) {
      declup -= 5;
      decllow += 5;
    }
  }
  decl = xx[1];
  // semi-diurnal arcs
  sda = -tan(dgeo[1] * DEGTORAD) * tan(decl * DEGTORAD);
  sdaup = -tan(dgeo[1] * DEGTORAD) * tan(declup * DEGTORAD);
  sdalow = -tan(dgeo[1] * DEGTORAD) * tan(decllow * DEGTORAD);
  if (sdalow <= -1) {
    if (serr)
      sprintf(serr, "rise or set not found for planet %d (1)", ipl);
    return -2; // is circumpolar
  }
  if (sdaup <= -1) 
    check_transits = TRUE;
  if (sdaup >= 1) {
    if (serr)
      sprintf(serr, "rise or set not found for planet %d (2)", ipl);
    return -2; // is circumpolar
  }
  if (sdalow >= 1)
    check_transits = TRUE;
  if (sda >= 1) {
    sda = 10;  // actually sda = 0°, but we give it a value of 10° 
               // to account for refraction. value 0 would cause
               // problems
  } else if (sda <= -1) {
    sda = 180;
  } else {
    sda = acos(sda) * RADTODEG;
  }
  // sidereal time at tjd_start
  armc = swe_degnorm(swe_sidtime(tjd_ut) * 15 + dgeo[0]); 
  // meridian distance of object
  md = swe_degnorm(xx[0] - armc);
  mdrise = swe_degnorm(sda * facrise);
  //dmd = swe_degnorm(md - mdrise - 1);
  dmd = swe_degnorm(md - mdrise);
  /* we search forward, but if we are close to a rising we may 
   * miss it. In this case we take the previous one and see
   * later if the resulting rising time is later than the input time */
/*if (0 && !is_second_run) {
    if (ipl == SE_MOON) {
      if (dmd > 350) 
	dmd -= 360;
    } else {
      if (dmd > 358) 
	dmd -= 360;
    }
  } */
  // rough subsequent rising/setting time
  tr = tjd_ut + dmd / 360;
  /* if object is sun or moon and rising of upper limb is required,
   * calculate apparent radius of disk (ignoring refraction);
   * with other objects disk diameter is ignored. */
  rdi = 0;
  /* true altitude of sun, when it appears at the horizon; 
   * refraction for a body visible at the horizon at 0m above sea,
   */
  if (atpress == 0) {
    /* estimate atmospheric pressure */
    atpress = 1013.25 * pow(1 - 0.0065 * dgeo[2] / 288, 5.255);
  } 
  swe_refrac_extended(0.000001, 0, atpress, attemp, const_lapse_rate, SE_APP_TO_TRUE, xx);
  refr = xx[1] - xx[0];
//fprintf(stderr, "refr=%f, %f, %f\n", refr, xx[0], xx[1]);
  swe_set_topo(dgeo[0], dgeo[1], dgeo[2]);
  // in doubtful cases check visibility of object at north and south meridian
  if (check_transits) {
    // north meridian
    double t, dmd2, md2, armc2, mdrise2, rdi2, dh;
    mdrise2 = facrise * 180;
    dmd2 = swe_degnorm(md - mdrise);
    dmd2 -= (180 - sda);
    t = tjd_ut + dmd2 / 360;
    for (i = 0; i < 2; i++) {
      if (swe_calc_ut(t, ipl, iflagtopo, xx, serr) == ERR)
	return ERR;
      armc2 = swe_degnorm(swe_sidtime(t) * 15 + dgeo[0]); 
      md2 = swe_degnorm(xx[0] - armc2);
      dmd2 = swe_degnorm(md2 - mdrise2);
      if (dmd2 > 180) dmd2 -= 360;
      t += dmd2 / 360;
    }
    if (swe_calc_ut(t, ipl, iflagtopo, xx, serr) == ERR)
      return ERR;
    swe_azalt(t, SE_EQU2HOR, dgeo, atpress, attemp, xx, xaz);
    rdi2 = get_sun_rad_plus_refr(ipl, xx[2], rsmi, refr); 
    dh = xaz[1] + rdi2;
    if (dh >= 0) {
      if (serr)
	sprintf(serr, "rise or set not found for planet %d (3)", ipl);
      return -2; // does not have a rising
    }
    // south meridian
    t += 0.5 * facrise;
    for (i = 0; i < 2; i++) {
      if (swe_calc_ut(t, ipl, iflagtopo, xx, serr) == ERR)
	return ERR;
      armc2 = swe_degnorm(swe_sidtime(t) * 15 + dgeo[0]); 
      md2 = swe_degnorm(xx[0] - armc2);
      if (md2 > 180) md2 -= 360;
      t += md2 / 360;
    }
    if (swe_calc_ut(t, ipl, iflagtopo, xx, serr) == ERR)
      return ERR;
    swe_azalt(t, SE_EQU2HOR, dgeo, atpress, attemp, xx, xaz);
    rdi2 = get_sun_rad_plus_refr(ipl, xx[2], rsmi, refr); 
    dh = xaz[1] + rdi2;
    if (dh <= 0) {
      if (serr) 
	sprintf(serr, "rise or set not found for planet %d (4)", ipl);
      return -2; // does not have a rising
    }
#if 0
    if (xaz[1] + rdi2 <= 0.2) {
      if (rsmi & SE_CALC_RISE) {
        sprintf(serr, "cannot find rising of planet %d near jd=%f; if you absolutely need one, try without flag SE_BIT_FAST_CALC", ipl, tr);
	return -2;
      }
      if (rsmi & SE_CALC_SET) {
        sprintf(serr, "cannot find setting of planet %d near jd=%f; if you absolutely need one, try without flag SE_BIT_FAST_CALC", ipl, tr);
	return -2;
      }
    }
#endif
  }
  if (is_fixstar) {
    if (swe_fixstar_ut(starname, tr, iflagtopo, xx, serr) == ERR)
      return ERR;
  } else {
    double dtsum = 0;
#if 0
    if (tjd_ut == 2451908.500000) {
      printf("hallo\n");
      fprintf(stderr, "rdi0=%f\n", rdi);
    }
#endif
    for (i = 0; i < nloop; i++) {
      if (swe_calc_ut(tr, ipl, iflagtopo, xx, serr) == ERR)
	return ERR;
      rdi = get_sun_rad_plus_refr(ipl, xx[2], rsmi, refr); 
      swe_azalt(tr, SE_EQU2HOR, dgeo, atpress, attemp, xx, xaz);
      swe_azalt(tr + 0.001, SE_EQU2HOR, dgeo, atpress, attemp, xx, xaz2);
      dd = (xaz2[1] - xaz[1]);
      dalt = xaz[1] + rdi;
      dt = dalt / dd / 1000.0;
      if (dt > 0.1) dt = 0.1;
      else if (dt < -0.1) dt = -0.1;
      dtsum += dt;
      if (fabs(dt) > 5.0 / 86400.0 && nloop < 20)
        nloop++;
      tr -= dt;
    }
//    if (tjd_ut == 2451780.500000)
//      fprintf(stderr, "dalt=%f\n", dalt);
    if (fabs(dalt) > 0.01) {
      if (serr)
	sprintf(serr, "rise or set not found for planet %d (5)", ipl);
      return -2;
    }
    if ((rsmi & SE_CALC_RISE) && xaz[0] < 180) {
      sprintf(serr, "cannot find rising of planet %d near jd=%f; if you absolutely need one, try without flag SE_BIT_FAST_CALC", ipl, tr);
      return -2;
    }
    if ((rsmi & SE_CALC_SET) && xaz[0] > 180) {
      sprintf(serr, "cannot find setting of planet %d near jd=%f; if you absolutely need one, try without flag SE_BIT_FAST_CALC", ipl, tr);
      return -2;
    }
    //  fprintf(stderr, "rdi=%f\n", rdi);
#if 0
    if (tjd_ut == 2451908.500000) {
      fprintf(stderr, "rdi=%f\n", rdi);
    }
    if (fabs(dtsum) > 1)
      fprintf(stderr, "dtsum=%f, nloop=%d\n", dtsum, nloop);
#endif
  }
//fprintf(stderr, "tr-tjd=%f tin=%f tout=%f\n", tr - tjd_ut, tjd_ut, tr);
  if (tr < tjd_ut0 && !is_second_run) {
    tjd_ut += 0.5; 
    is_second_run = TRUE;
    goto run_rise_again;
  } 
  *tret = tr;
  return OK;
}


static int32 calc_rise_and_set(double tjd_start, int32 ipl, char *starname, double *dgeo, double *datm, int32 eventflag, int32 helflag, double *trise, char *serr)
{
  int retc = OK, i;
  double xx[6], xaz[6], xaz2[6], dfac = 1/365.25;
  double rdi, rh, md, sda;
  double tjd0 = tjd_start, tjdrise;
  double armc;
//if (ipl == -1) return -2;
  if (eventflag & (SE_CALC_MTRANSIT|SE_CALC_ITRANSIT)) return -2;
//  if (ipl == SE_MOON) return -2;
//  if (fabs(dgeo[1]) > 63) return -2;
  int32 iflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  iflag |= SEFLG_EQUATORIAL;
  if (!(helflag & SE_HELFLAG_HIGH_PRECISION))
    iflag |= SEFLG_NONUT|SEFLG_TRUEPOS;
#if 0
  if (swe_calc_ut(tjd0, SE_SUN, iflag, xs, serr) == ERR) {
    if (serr != NULL)
      strcpy(serr, "error in calc_rise_and_set(): swe_calc(sun) failed ");
    return ERR;
  }
#endif
  if (ipl == -1) {
    if (swe_fixstar_ut(starname, tjd0, iflag, xx, serr) == ERR) {
      if (serr != NULL)
	sprintf(serr, "error in calc_rise_and_set(): swe_fixstar(%s) failed ", starname);
      return ERR;
    }
  } else {
    if (swe_calc_ut(tjd0, ipl, iflag, xx, serr) < 0) {
      if (serr != NULL)
	sprintf(serr, "error in calc_rise_and_set(): calc(planet %d) failed ", ipl);
      return ERR;
    }
  }
#if 0
  // diurnal arc of Sun
  sds = -tan(dgeo[1] * DEGTORAD) * tan(xs[1] * DEGTORAD);
  if (sds <= -1) sds = 180;
  else if (sds >= 1) sds = 0;
  else sds = acos(sda) * RADTODEG;
#endif
  // diurnal arc of object
  sda = -tan(dgeo[1] * DEGTORAD) * tan(xx[1] * DEGTORAD);
  if (fabs(sda) >= 1) 
    return -2;  // object is circumpolar
  sda = acos(sda) * RADTODEG;
  if (ipl ==  -1 && (sda > 179 || sda < 1))
    return -2;  // star is near circumpolar
  if (ipl == SE_MOON && (sda > 160 || sda < 20))
    return -2;  // moon is near circumpolar
  if (ipl > SE_SUN && (sda > 170 || sda < 10))
    return -2;  // moon is near circumpolar
  // sidereal time at tjd_start
  armc = swe_degnorm(swe_sidtime(tjd_start) * 15 + dgeo[0]);
#if 0
  // is day or night
  if (swe_degnorm(xs[0] - armc) <= sds || swe_degnorm(armc - xs[0]) <= sds)
    is_day = TRUE;
  else
    is_day = FALSE;
#endif
  // meridian distance of object
  md = swe_degnorm(xx[0] - armc);
  if (md > 180) md -= 360;
  // above horizon, before culmination
  if (md >= 0 && md <= sda) {
    tjdrise = tjd_start - (sda - md) / 360.0;
    if (eventflag & SE_CALC_RISE)  	// approx. rising time
      tjdrise += 1.0;
    else  		           	// approx. setting time
      tjdrise += 0.5;
  // above horizon, after culmination
  } else if (md < 0 && md >= -sda) {
    tjdrise = tjd_start + (sda + md) / 360.0;
    if (eventflag & SE_CALC_RISE)	// approx. rising time
      tjdrise += 0.5;
    else				// approx. setting time
      ;
  // below horizon, after lower culmination
  } else if (md >= 0 && md > sda) {
    tjdrise = tjd_start + (md - sda) / 360.0;
    if (eventflag & SE_CALC_RISE)	// approx. rising time
      ;
    else				// approx. setting time
      tjdrise += 0.5;
  // below horizon, before lower culmination
  } else {
    tjdrise = tjd_start - (sda + md) / 360.0;
    if (eventflag & SE_CALC_RISE)	// approx. rising time
      tjdrise += 0.5;
    else				// approx. setting time
      tjdrise += 1.0;
  }
  /* if object is sun and rising of upper limb is required,
   * calculate apparent radius of solar disk (ignoring refraction);
   * with other objects disk diameter is ignored. 
   * This function must not be called for the moon! */
  rdi = 0;
  if (ipl == SE_SUN) 
    rdi = asin(696000000.0 / 1.49597870691e+11 / xx[2]) / DEGTORAD;
  if (eventflag & SE_BIT_DISC_CENTER)
    rdi = 0;
  /* true altitude of sun, when it appears at the horizon */
  /* refraction for a body visible at the horizon at 0m above sea,
   * atmospheric temperature 10degC, atmospheric pressure 1013.25 is 34.5 arcmin*/
  rh = -rdi;
  if (!(eventflag & SE_BIT_NO_REFRACTION))
    rh = -(34.5 / 60.0 + rdi);
  /* now calculate more accurate rising and setting times.
   * use vertical speed in order to determine crossing of the horizon
   * refraction of 34' and solar disk diameter of 16' = 50' = 0.84 deg */
  iflag = SEFLG_SPEED|SEFLG_EQUATORIAL;
  if (ipl == SE_MOON)
    iflag |= SEFLG_TOPOCTR;
  if (!(helflag & SE_HELFLAG_HIGH_PRECISION))
    iflag |= SEFLG_NONUT|SEFLG_TRUEPOS;
  if (ipl >= SE_SUN) {
    for (i = 0; i < 2; i++) {
      if (ipl == -1) {
	if (swe_fixstar_ut(starname, tjdrise, iflag, xx, serr) == ERR) 
	  return ERR;
      } else {
	if (swe_calc_ut(tjdrise, ipl, iflag, xx, serr) == ERR)
	  return ERR;
      }
      swe_azalt(tjdrise, SE_EQU2HOR, dgeo, datm[0], datm[1], xx, xaz);
      xx[0] -= xx[3] * dfac;
      xx[1] -= xx[4] * dfac;
      swe_azalt(tjdrise - dfac, SE_EQU2HOR, dgeo, datm[0], datm[1], xx, xaz2);
      tjdrise -= (xaz[1] - rh) / (xaz[1] - xaz2[1]) *  dfac;
      /*fprintf(stderr, "%f\n", ph->trise);*/
    }
  }
  *trise = tjdrise;
  return retc;
}

#else 
static int32 calc_rise_and_set_test(double tjd_start, int32 ipl, char *starname, double *dgeo, double *datm, int32 eventflag, int32 helflag, double *trise, char *serr)
{
  int retc = OK, i;
  double sda, xs[6], xx[6], xaz[6], xaz2[6], dfac = 1/365.25;
  double rdi, rh;
  double tjd0 = tjd_start, tjdrise;
  double tjdnoon = (int) tjd0 - dgeo[0] / 15.0 / 24.0;
//if (ipl == -1) return -2;
  if (eventflag & (SE_CALC_MTRANSIT|SE_CALC_ITRANSIT)) return -2;
//  if (ipl == SE_MOON) return -2;
//  if (fabs(dgeo[1]) > 63) return -2;
  int32 iflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  iflag |= SEFLG_EQUATORIAL;
  if (!(helflag & SE_HELFLAG_HIGH_PRECISION))
    iflag |= SEFLG_NONUT|SEFLG_TRUEPOS;
  if (swe_calc_ut(tjd0, SE_SUN, iflag, xs, serr) == ERR) {
    if (serr != NULL)
      strcpy(serr, "error in calc_rise_and_set(): swe_calc(sun) failed ");
    return ERR;
  }
  if (ipl == -1) {
    if (swe_fixstar_ut(starname, tjd0, iflag, xx, serr) == ERR) {
      if (serr != NULL)
	sprintf(serr, "error in calc_rise_and_set(): swe_fixstar(%s) failed ", starname);
      return ERR;
    }
  } else {
    if (swe_calc_ut(tjd0, ipl, iflag, xx, serr) < 0) {
      if (serr != NULL)
	sprintf(serr, "error in calc_rise_and_set(): calc(planet %d) failed ", ipl);
      return ERR;
    }
  }
  sda = -tan(dgeo[1] * DEGTORAD) * tan(xx[1] * DEGTORAD);
  if (fabs(sda) >= 1) 
    return -2;  // object is circumpolar
  sda = acos(sda) * RADTODEG;
  if (ipl ==  -1 && (sda > 179 || sda < 1))
    return -2;  // star is near circumpolar
  if (ipl == SE_MOON && (sda > 160 || sda < 20))
    return -2;  // moon is near circumpolar
  if (ipl > SE_SUN && (sda > 170 || sda < 10))
    return -2;  // moon is near circumpolar
  tjdnoon -= swe_degnorm(xs[0] - xx[0])/360.0 + 0;
  /* is planet above horizon or below? */
  swe_azalt(tjd0, SE_EQU2HOR, dgeo, datm[0], datm[1], xx, xaz);
if (eventflag & SE_CALC_RISE) {
  if (xaz[2] > 0) {
    while (tjdnoon - tjd0 < 0.5) {/*printf("e");*/tjdnoon += 1;}
    while (tjdnoon - tjd0 > 1.5) {/*printf("f");*/tjdnoon -= 1;}
  } else {
    while (tjdnoon - tjd0 < 0.0) {/*printf("g");*/tjdnoon += 1;}
    while (tjdnoon - tjd0 > 1.0) {/*printf("h");*/tjdnoon -= 1;}
  }
} else {
  if (xaz[2] > 0) {
    while (tjd0 - tjdnoon > 0.5) {/*printf("a");*/ tjdnoon += 1;}
    while (tjd0 - tjdnoon < -0.5) {/*printf("b");*/ tjdnoon -= 1;}
  } else {
    while (tjd0 - tjdnoon > 0.0) {/*printf("c");*/ tjdnoon += 1;}
    while (tjd0 - tjdnoon < -1.0) {/*printf("d");*/ tjdnoon -= 1;}
  }
}
  /* position of planet */
  if (ipl == -1) {
    if (swe_fixstar_ut(starname, tjdnoon, iflag, xx, serr) == ERR) {
      if (serr != NULL)
	sprintf(serr, "error in calc_rise_and_set(): swe_fixstar(%s) failed ", starname);
      return ERR;
    }
  } else {
    if (swe_calc_ut(tjdnoon, ipl, iflag, xx, serr) == ERR) {
      if (serr != NULL)
	strcpy(serr, "error in calc_rise_and_set(): swe_calc(sun) failed ");
      return ERR;
    }
  }
  /* if object is sun and rising of upper limb is required,
   * calculate apparent radius of solar disk (ignoring refraction);
   * with other objects disk diameter is ignored. 
   * This function must not be called for the moon! */
  rdi = 0;
  if (ipl == SE_SUN) 
    rdi = asin(696000000.0 / 1.49597870691e+11 / xx[2]) / DEGTORAD;
  if (eventflag & SE_BIT_DISC_CENTER)
    rdi = 0;
  /* true altitude of sun, when it appears at the horizon */
  /* refraction for a body visible at the horizon at 0m above sea,
   * atmospheric temperature 10degC, atmospheric pressure 1013.25 is 34.5 arcmin*/
  rh = -rdi;
  if (!(eventflag & SE_BIT_NO_REFRACTION))
    rh = -(34.5 / 60.0 + rdi);
  /* semidiurnal arc of sun */
  sda = acos(-tan(dgeo[1] * DEGTORAD) * tan(xx[1] * DEGTORAD)) * RADTODEG;
  /* rough rising and setting times */
if (eventflag & SE_CALC_RISE)
  tjdrise = tjdnoon - sda / 360.0;
else
  tjdrise = tjdnoon + sda / 360.0;
  /*ph->tset = tjd_start + sda / 360.0;*/
  /* now calculate more accurate rising and setting times.
   * use vertical speed in order to determine crossing of the horizon
   * refraction of 34' and solar disk diameter of 16' = 50' = 0.84 deg */
  iflag = SEFLG_SPEED|SEFLG_EQUATORIAL;
  if (ipl == SE_MOON)
    iflag |= SEFLG_TOPOCTR;
  if (!(helflag & SE_HELFLAG_HIGH_PRECISION))
    iflag |= SEFLG_NONUT|SEFLG_TRUEPOS;
  if (ipl >= SE_SUN) {
    for (i = 0; i < 2; i++) {
      if (swe_calc_ut(tjdrise, ipl, iflag, xx, serr) == ERR)
	return ERR;
      swe_azalt(tjdrise, SE_EQU2HOR, dgeo, datm[0], datm[1], xx, xaz);
      xx[0] -= xx[3] * dfac;
      xx[1] -= xx[4] * dfac;
      swe_azalt(tjdrise - dfac, SE_EQU2HOR, dgeo, datm[0], datm[1], xx, xaz2);
      tjdrise -= (xaz[1] - rh) / (xaz[1] - xaz2[1]) *  dfac;
      /*fprintf(stderr, "%f\n", ph->trise);*/
    }
  }
  *trise = tjdrise;
  return retc;
}
