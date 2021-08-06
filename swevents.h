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
