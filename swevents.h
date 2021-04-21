/* Copyright (C) 1997 - 2021 Astrodienst AG, Switzerland.  All rights reserved.

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
  a) GNU Affero General Public License (AGPL)
  b) Swiss Ephemeris Professional License

  The choice must be made before the software developer distributes software
  containing parts of Swiss Ephemeris to others, and before any public
  service using the developed software is activated.

  If the developer choses the AGPL software license, he or she must fulfill
  the conditions of that license, which includes the obligation to place his
  or her whole software project under the AGPL or a compatible license.
  See https://www.gnu.org/licenses/agpl-3.0.html

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
#define TTINY 0.000001
#define TENTHOFSEC  (1.0 / 86400.0)
#define HUNDTHOFSEC  (1.0 / 864000.0)

#define NEVENTMAX 100000
#define EVENT struct event
struct event {
  double tjd;
  int32  evtype;
  int32  ipla;
  int32  iplb;                  /* needed for aspects, occultations, */
  char stnama[40];
  char stnamb[40];
  int32  iasp;
  int32  bpind;  /* body pair index: ipl1 * 50 + ipl2 */
  double  dasp;
  double  dang;
  int32  isign;                 /* needed for ingresses */
  int32  backward;              /* needed for ingresses */
  double dorb;
  double dret;              /* content depends on event type */
};

struct aspdat {
  double tjd;
  int32 iasp;
  double tjd_pre;
  double tjd_post;
  int32 fpos_tjd_post;
};

#define CTYP_MASPECTS	0
#define CTYP_INGRESSES	1
#define CTYP_TRANSITS	2
#define CTYP_VOC	3

#define SPLAN_INGRESS   "0123456789mtAFD"
#define SPLAN_ASPECTS   "0123456789mtAFD,a[136199],f[Gal]"
/*#define SPLAN_ASPECTS   "0123456789mtAFD,f[Gal],f[Ald],a[136199],a[433]"*/
#define SASP_ASPECTS   "1234567"

/* void-of-course moon calculation */
#define VOC		struct voc

VOC {
  double tvoc;    /* time of begin of voc phase */
  double tingr;   /* time of ingress that ends voc phase */
  double tingr0;  /* time of first ingress, if there are two ingresses
                   * during voc phase */
  char casp, cpl; /* aspect and planet that marks begin of voc phase */
  int isign_voc;  /* sign in which moon begins voc phase */
  int isign_ingr; /* sign of ingress that ends voc phase */
  int isign_ingr0; /* sign of first ingress, if there are two ingresses
                    * during voc phase */
};

#define INGRESS	struct ingress

INGRESS {
  int ipl;	  /* planet that makes ingress */
  double tingr;   /* time of ingress */
  int isign;      /* sign of ingress */
  int direction;  /* direct/retrograde: 1 or -1 */
  int ino;	  /* number of transit over this sign boundary */
};
