/*
 * Author    Yura Krymlov
 * Created   2021-02
 * Version   2.10.03
 */
 
#include "swephexp.h"
#include "swejni.h"

#define JNI_RELEASE (ERR >= retc ? JNI_ABORT : JNI_OK)

#define DEFINE_CHAR_SERR char serr[AS_MAXCH];
#define ERR_BUILDER_APPEND_IF_ERR if (ERR >= retc && NULL != errBuilder) appendToBuilder(env, serr, errBuilder);
#define ERR_BUILDER_APPEND_IF_SERR if (*serr != '\0' && NULL != errBuilder) appendToBuilder(env, serr, errBuilder);

#define GET_DOUBLE_ARRAY_ELEMENTS(IS_COPY, JDOUBLE_ARRAY, ELEMENTS)\
    jboolean IS_COPY = JNI_FALSE;                                              \
    jdouble* ELEMENTS = (NULL != JDOUBLE_ARRAY) ? (*env)->GetDoubleArrayElements(env, JDOUBLE_ARRAY, &IS_COPY) : NULL;

#define RLZ_DOUBLE_ARRAY_ELEMENTS_OK(IS_COPY, JDOUBLE_ARRAY, ELEMENTS)\
    if (JNI_TRUE == IS_COPY && NULL != JDOUBLE_ARRAY) (*env)->ReleaseDoubleArrayElements(env, JDOUBLE_ARRAY, ELEMENTS, JNI_OK);

#define RLZ_DOUBLE_ARRAY_ELEMENTS(IS_COPY, JDOUBLE_ARRAY, ELEMENTS)\
    if (JNI_TRUE == IS_COPY && NULL != JDOUBLE_ARRAY) (*env)->ReleaseDoubleArrayElements(env, JDOUBLE_ARRAY, ELEMENTS, JNI_RELEASE);

#define GET_INT_ARRAY_ELEMENTS(IS_COPY, JINT_ARRAY, ELEMENTS) \
    jboolean IS_COPY = JNI_FALSE;                                         \
    jint * ELEMENTS = (NULL != JINT_ARRAY) ? (*env)->GetIntArrayElements(env, JINT_ARRAY, &IS_COPY) : NULL;

#define RLZ_INT_ARRAY_ELEMENTS_OK(IS_COPY, JINT_ARRAY, ELEMENTS)\
    if (JNI_TRUE == IS_COPY && NULL != JINT_ARRAY) (*env)->ReleaseIntArrayElements(env, JINT_ARRAY, ELEMENTS, JNI_OK);

#define GET_STRING_UTF_CHARS(IS_COPY, JSTRING, CCSTRING)\
    jboolean IS_COPY = JNI_FALSE;                                   \
    const char* CCSTRING = (NULL != JSTRING) ? (*env)->GetStringUTFChars(env, JSTRING, &IS_COPY) : NULL;

#define RLZ_STRING_UTF_CHARS(IS_COPY, JSTRING, CCSTRING)\
    if (JNI_TRUE == IS_COPY && NULL != JSTRING) (*env)->ReleaseStringUTFChars(env, JSTRING, CCSTRING);

#define CPY_CSTRING_TO_CHARS(CCSTRING, CHARS)    \
    char CHARS[AS_MAXCH];                        \
    if (NULL == CCSTRING) *CHARS = '\0';         \
    else strcpy(CHARS, CCSTRING);

#define BUILDER_APPEND_IF_DIFF(BUILDER, CCSTRING, CHARS)    \
    if (NULL != BUILDER && NULL != CCSTRING && 0 != strcmp(CCSTRING, CHARS)) { \
        emptyBuilder(env, BUILDER);                         \
        appendToBuilder(env, CHARS, BUILDER);               \
    }

/*
 * JNIEXPORT void JNICALL Java_package_class_method
 * (JNIEnv *env, jclass clazz, jdoubleArray dblArray) {
 *      // The isArrayCopy variable boolean indicates whether
 *      // the copy of the primitive elements was a copy or not.
 *      jboolean isArrayCopy;
 *
 *      // If it is a copy, one must remember to free it before returning.
 *      jdouble* dblElements = (*env)->GetDoubleArrayElements(env, dblArray, &isArrayCopy);
 *
 *      // Also, if we made any changes to the array, we must copy it back to
 *      // the original one so that the changes are reflected in the original.
 *      // This is the purpose of the ReleaseDoubleArrayElements call.
 *      if (isArrayCopy == JNI_TRUE) (*env)->ReleaseDoubleArrayElements(env, dblArray, dblElements, JNI_OK);
 *
 *      Release Modes:
 *      JNI_OK      Copy the contents of the buffer back into array and free the buffer
 *      JNI_ABORT	Free the buffer without copying back any changes
 *      JNI_COMMIT	Copy the contents of the buffer back into array but do not free buffer
 * }
*/

jstring getBuilderString(JNIEnv *env, jobject builder) {
    if (NULL == builder) return NULL;

    // Obtain the Java StringBuilder class handle
    jclass clazz = (*env)->GetObjectClass(env, builder);

    // Obtain the method ID for the StringBuilder toString method
    jmethodID mid = (*env)->GetMethodID(env, clazz, "toString", "()Ljava/lang/String;");

    // If this method does not exist then return.
    if (mid == 0) return NULL;

    // Call the StringBuilder object's toString method
    return (*env)->CallObjectMethod(env, builder, mid);
}

void emptyBuilder(JNIEnv *env, jobject builder) {
    if (NULL == builder) return;

    // Obtain the Java StringBuilder class handle
    jclass clazz = (*env)->GetObjectClass(env, builder);

    // Obtain the method ID for the StringBuilder setLength method
    jmethodID mid = (*env)->GetMethodID(env, clazz, "setLength", "(I)V");

    // If this method does not exist then return.
    if (mid == 0) return;

    // Call the StringBuilder object's setLength method
    (*env)->CallVoidMethod(env, builder, mid, 0);
}

void appendToBuilder(JNIEnv *env, char *chArray, jobject builder) {
    if (NULL == builder) return;

    // Obtain the Java StringBuilder class handle
    jclass clazz = (*env)->GetObjectClass(env, builder);

    // Obtain the method ID for the StringBuilder append method
    jmethodID mid = (*env)->GetMethodID(env, clazz, "append", "(Ljava/lang/String;)Ljava/lang/StringBuilder;");

    // If this method does not exist then return.
    if (mid == 0) return;

    // Create a new Java String object for the given char Array
    jstring jArray = (*env)->NewStringUTF(env, chArray);

    // Call the StringBuilder object's append method
    (*env)->CallObjectMethod(env, builder, mid, jArray);
}

/*
 * int32 swe_heliacal_ut(double tjdstart_ut, double *geopos, double *datm, double *dobs,
 * char *ObjectName, int32 TypeEvent, int32 iflag, double *dret, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_heliacal_ut
 * Signature: (D[D[D[DLjava/lang/String;II[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1heliacal_1ut(JNIEnv *env, jclass swephexp,
                                         jdouble tjdstart_ut, jdoubleArray geoposArray, jdoubleArray datmArray,
                                         jdoubleArray dobsArray, jstring objectName, jint typeEvent, jint iflag,
                                         jdoubleArray dretArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, datmArray, datm)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy3, dobsArray, dobs)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy4, dretArray, dret)

    GET_STRING_UTF_CHARS(isCopy, objectName, objName)
    CPY_CSTRING_TO_CHARS(objName, objectNameIn)

    int32 retc = swe_heliacal_ut(tjdstart_ut, geopos, datm, dobs, objectNameIn,
                                 typeEvent, iflag, dret, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy1, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy2, datmArray, datm)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy3, dobsArray, dobs)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy4, dretArray, dret)
    RLZ_STRING_UTF_CHARS(isCopy, objectName, objName)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_heliacal_pheno_ut(double tjd_ut, double *geopos, double *datm, double *dobs,
 *  char *ObjectName, int32 TypeEvent, int32 helflag, double *darr, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_heliacal_pheno_ut
 * Signature: (D[D[D[DLjava/lang/String;II[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1heliacal_1pheno_1ut(JNIEnv *env, jclass swephexp,
                                                jdouble tjd_ut, jdoubleArray geoposArray,
                                                jdoubleArray datmArray, jdoubleArray dobsArray,
                                                jstring objectName, jint typeEvent, jint helflag,
                                                jdoubleArray darrArray, jobject errBuilder) {
    DEFINE_CHAR_SERR

    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, datmArray, datm)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy3, dobsArray, dobs)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy4, darrArray, darr)

    GET_STRING_UTF_CHARS(isCopy, objectName, objName)
    CPY_CSTRING_TO_CHARS(objName, objectNameIn)

    int32 retc = swe_heliacal_pheno_ut(tjd_ut, geopos, datm, dobs, objectNameIn, typeEvent, helflag, darr, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy1, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy2, datmArray, datm)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy3, dobsArray, dobs)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy4, darrArray, darr)
    RLZ_STRING_UTF_CHARS(isCopy, objectName, objName)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_vis_limit_mag(double tjdut, double *geopos, double *datm, double *dobs,
 *      char *ObjectName, int32 helflag, double *dret, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_vis_limit_mag
 * Signature: (D[D[D[DLjava/lang/StringBuilder;I[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1vis_1limit_1mag(JNIEnv *env, jclass swephexp,
                                            jdouble tjdut, jdoubleArray geoposArray, jdoubleArray datmArray,
                                            jdoubleArray dobsArray, jobject objectNameBuilder, jint helflag,
                                            jdoubleArray dretArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, datmArray, datm)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy3, dobsArray, dobs)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy4, dretArray, dret)

    jstring objectName = getBuilderString(env, objectNameBuilder);
    GET_STRING_UTF_CHARS(isCopy, objectName, objectname)
    CPY_CSTRING_TO_CHARS(objectname, object_name)

    int32 retc = swe_vis_limit_mag(tjdut, geopos, datm, dobs, object_name, helflag, dret, serr);

    BUILDER_APPEND_IF_DIFF(objectNameBuilder, objectname, object_name)

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy1, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy2, datmArray, datm)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy3, dobsArray, dobs)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy4, dretArray, dret)
    RLZ_STRING_UTF_CHARS(isCopy, objectName, objectname)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_heliacal_angle(double tjdut, double *dgeo, double *datm, double *dobs,
 *      int32 helflag, double mag, double azi_obj, double azi_sun, double azi_moon,
 *          double alt_moon, double *dret, char *serr);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_heliacal_angle
 * Signature: (D[D[D[DIDDDDD[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1heliacal_1angle(JNIEnv *env, jclass swephexp,
                                            jdouble tjdut, jdoubleArray dgeoArray,
                                            jdoubleArray datmArray, jdoubleArray dobsArray, jint helflag,
                                            jdouble mag, jdouble azi_obj, jdouble azi_sun, jdouble azi_moon,
                                            jdouble alt_moon, jdoubleArray dretArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, dgeoArray, dgeo)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, datmArray, datm)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy3, dobsArray, dobs)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy4, dretArray, dret)

    int32 retc = swe_heliacal_angle(tjdut, dgeo, datm, dobs,helflag, mag, azi_obj,
                                    azi_sun, azi_moon,alt_moon, dret, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy1, dgeoArray, dgeo)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy2, datmArray, datm)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy3, dobsArray, dobs)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy4, dretArray, dret)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_topo_arcus_visionis(double tjdut, double *dgeo, double *datm, double *dobs,
 *      int32 helflag, double mag, double azi_obj, double alt_obj, double azi_sun,
 *          double azi_moon, double alt_moon, double *dret, char *serr);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_topo_arcus_visionis
 * Signature: (D[D[D[DIDDDDDD[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1topo_1arcus_1visionis(JNIEnv *env, jclass swephexp,
                                                  jdouble tjdut, jdoubleArray dgeoArray,
                                                  jdoubleArray datmArray, jdoubleArray dobsArray,
                                                  jint helflag, jdouble mag, jdouble azi_obj, jdouble alt_obj,
                                                  jdouble azi_sun, jdouble azi_moon, jdouble alt_moon,
                                                  jdoubleArray dretArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, dgeoArray, dgeo)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, datmArray, datm)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy3, dobsArray, dobs)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy4, dretArray, dret)

    int32 retc = swe_topo_arcus_visionis(tjdut, dgeo, datm, dobs,helflag, mag, azi_obj,
                                         alt_obj, azi_sun,azi_moon, alt_moon, dret, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy1, dgeoArray, dgeo)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy2, datmArray, datm)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy3, dobsArray, dobs)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy4, dretArray, dret)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * void swe_set_astro_models(char *samod, int32 iflag)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_set_astro_models
 * Signature: (Ljava/lang/StringBuilder;I)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1set_1astro_1models(JNIEnv *env, jclass swephexp,
                                               jobject samodBuilder, jint iflag) {
    jstring samodObject = getBuilderString(env, samodBuilder);
    GET_STRING_UTF_CHARS(isCopy, samodObject, samodobject)
    CPY_CSTRING_TO_CHARS(samodobject, samod_object)

    swe_set_astro_models(samod_object, iflag);

    BUILDER_APPEND_IF_DIFF(samodBuilder, samodobject, samod_object)
    RLZ_STRING_UTF_CHARS(isCopy, samodObject, samodobject)
}

/*
 * void swe_get_astro_models(char *samod, char *sdet, int32 iflag)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_get_astro_models
 * Signature: (Ljava/lang/StringBuilder;Ljava/lang/StringBuilder;I)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1get_1astro_1models(JNIEnv *env, jclass swephexp,
                                               jobject samodBuilder, jobject sdetBuilder, jint iflag) {
    jstring samodObject = getBuilderString(env, samodBuilder);
    GET_STRING_UTF_CHARS(isCopy1, samodObject, samodobject)
    CPY_CSTRING_TO_CHARS(samodobject, samod_object)

    jstring sdetObject = getBuilderString(env, sdetBuilder);
    GET_STRING_UTF_CHARS(isCopy2, sdetObject, sdetobject)
    CPY_CSTRING_TO_CHARS(sdetobject, sdet_object)

    swe_get_astro_models(samod_object, sdet_object, iflag);

    BUILDER_APPEND_IF_DIFF(samodBuilder, samodobject, samod_object)
    BUILDER_APPEND_IF_DIFF(sdetBuilder, sdetobject, sdet_object)

    RLZ_STRING_UTF_CHARS(isCopy1, samodObject, samodobject)
    RLZ_STRING_UTF_CHARS(isCopy2, sdetObject, sdetobject)
}

/*
 * char* swe_version(char *)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_version
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_swisseph_SwephExp_swe_1version(JNIEnv *env, jclass swephexp) {
    char s[AS_MAXCH];
    return (*env)->NewStringUTF(env, swe_version(s));
}

/*
 * char * swe_get_library_path(char *)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_get_library_path
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_swisseph_SwephExp_swe_1get_1library_1path(JNIEnv *env, jclass swephexp) {
    char s[AS_MAXCH];
    return (*env)->NewStringUTF(env, swe_get_library_path(s));
}

/*
 * int swe_calc(double tjd, int ipl, int32 iflag, double *xx, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_calc
 * Signature: (DII[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1calc(JNIEnv *env, jclass swephexp, jdouble tjd, jint ipl,
                                 jint iflag, jdoubleArray xxArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy, xxArray, xx)

    int32 retc = swe_calc(tjd, ipl, iflag, xx, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy, xxArray, xx)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_calc_ut
 * Signature: (DII[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1calc_1ut(JNIEnv *env, jclass swephexp, jdouble tjd_ut, jint ipl,
                                     jint iflag, jdoubleArray xxArray, jobject errBuilder) {

    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy, xxArray, xx)

    int32 retc = swe_calc_ut(tjd_ut, ipl, iflag, xx, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy, xxArray, xx)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_calc_pctr(double tjd, int32 ipl, int32 iplctr, int32 iflag, double *xxret, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_calc_pctr
 * Signature: (DIII[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1calc_1pctr(JNIEnv *env, jclass swephexp, jdouble tjd, jint ipl,
                                       jint iplctr, jint iflag, jdoubleArray xxretArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy, xxretArray, xxret)

    int32 retc = swe_calc_pctr(tjd, ipl, iplctr, iflag, xxret, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy, xxretArray, xxret)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_fixstar(char *star, double tjd, int32 iflag,double *xx,char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_fixstar
 * Signature: (Ljava/lang/StringBuilder;DI[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1fixstar(JNIEnv *env, jclass swephexp, jobject starNameBuilder, jdouble tjd,
                                    jint iflag, jdoubleArray xxArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, xxArray, xx)

    jstring starName = getBuilderString(env, starNameBuilder);
    GET_STRING_UTF_CHARS(isCopy, starName, starname)
    CPY_CSTRING_TO_CHARS(starname, star_name)

    int32 retc = swe_fixstar(star_name, tjd, iflag, xx, serr);

    BUILDER_APPEND_IF_DIFF(starNameBuilder, starname, star_name)

    RLZ_DOUBLE_ARRAY_ELEMENTS(isDblCopy, xxArray, xx)
    RLZ_STRING_UTF_CHARS(isCopy, starName, starname)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_fixstar_ut(char *star, double tjd_ut, int32 iflag, double *xx, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_fixstar_ut
 * Signature: (Ljava/lang/StringBuilder;DI[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1fixstar_1ut(JNIEnv *env, jclass swephexp, jobject starNameBuilder, jdouble tjd_ut,
                                        jint iflag, jdoubleArray xxArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, xxArray, xx)

    jstring starName = getBuilderString(env, starNameBuilder);
    GET_STRING_UTF_CHARS(isCopy, starName, starname)
    CPY_CSTRING_TO_CHARS(starname, star_name)

    int32 retc = swe_fixstar_ut(star_name, tjd_ut, iflag, xx, serr);

    BUILDER_APPEND_IF_DIFF(starNameBuilder, starname, star_name)

    RLZ_DOUBLE_ARRAY_ELEMENTS(isDblCopy, xxArray, xx)
    RLZ_STRING_UTF_CHARS(isCopy, starName, starname)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_fixstar_mag(char *star, double *mag, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_fixstar_mag
 * Signature: (Ljava/lang/StringBuilder;[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1fixstar_1mag(JNIEnv *env, jclass swephexp, jobject starNameBuilder,
                                         jdoubleArray magArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, magArray, mag)

    jstring starName = getBuilderString(env, starNameBuilder);
    GET_STRING_UTF_CHARS(isCopy, starName, starname)
    CPY_CSTRING_TO_CHARS(starname, star_name)

    int32 retc = swe_fixstar_mag(star_name, mag, serr);

    BUILDER_APPEND_IF_DIFF(starNameBuilder, starname, star_name)

    RLZ_DOUBLE_ARRAY_ELEMENTS(isDblCopy, magArray, mag)
    RLZ_STRING_UTF_CHARS(isCopy, starName, starname)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 *  int32 swe_fixstar2(char *star, double tjd, int32 iflag,double *xx,char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_fixstar2
 * Signature: (Ljava/lang/StringBuilder;DI[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1fixstar2(JNIEnv *env, jclass swephexp, jobject starNameBuilder, jdouble tjd,
                                     jint iflag, jdoubleArray xxArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, xxArray, xx)

    jstring starName = getBuilderString(env, starNameBuilder);
    GET_STRING_UTF_CHARS(isCopy, starName, starname)
    CPY_CSTRING_TO_CHARS(starname, star_name)

    int32 retc = swe_fixstar2(star_name, tjd, iflag, xx, serr);

    BUILDER_APPEND_IF_DIFF(starNameBuilder, starname, star_name)

    RLZ_DOUBLE_ARRAY_ELEMENTS(isDblCopy, xxArray, xx)
    RLZ_STRING_UTF_CHARS(isCopy, starName, starname)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_fixstar2_ut(char *star, double tjd_ut, int32 iflag, double *xx, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_fixstar2_ut
 * Signature: (Ljava/lang/StringBuilder;DI[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1fixstar2_1ut(JNIEnv *env, jclass swephexp, jobject starNameBuilder, jdouble tjd_ut,
                                         jint iflag, jdoubleArray xxArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, xxArray, xx)

    jstring starName = getBuilderString(env, starNameBuilder);
    GET_STRING_UTF_CHARS(isCopy, starName, starname)
    CPY_CSTRING_TO_CHARS(starname, star_name)

    int32 retc = swe_fixstar2_ut(star_name, tjd_ut, iflag, xx, serr);

    BUILDER_APPEND_IF_DIFF(starNameBuilder, starname, star_name)

    RLZ_DOUBLE_ARRAY_ELEMENTS(isDblCopy, xxArray, xx)
    RLZ_STRING_UTF_CHARS(isCopy, starName, starname)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_fixstar2_mag(char *star, double *mag, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_fixstar2_mag
 * Signature: (Ljava/lang/StringBuilder;[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1fixstar2_1mag(JNIEnv *env, jclass swephexp, jobject starNameBuilder,
                                          jdoubleArray magArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, magArray, mag)

    jstring starName = getBuilderString(env, starNameBuilder);
    GET_STRING_UTF_CHARS(isCopy, starName, starname)
    CPY_CSTRING_TO_CHARS(starname, star_name)

    int32 retc = swe_fixstar2_mag(star_name, mag, serr);

    BUILDER_APPEND_IF_DIFF(starNameBuilder, starname, star_name)

    RLZ_DOUBLE_ARRAY_ELEMENTS(isDblCopy, magArray, mag)
    RLZ_STRING_UTF_CHARS(isCopy, starName, starname)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_close
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1close(JNIEnv *env, jclass swephexp) {
    swe_close();
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_set_ephe_path
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1set_1ephe_1path(JNIEnv *env, jclass swephexp, jstring jEphePath) {
    GET_STRING_UTF_CHARS(isCopy, jEphePath, ephePath)
    //CPY_CSTRING_TO_CHARS(ephePath, ephe_path) SINCE: 2.10.03

    swe_set_ephe_path(ephePath);

    RLZ_STRING_UTF_CHARS(isCopy, jEphePath, ephePath)
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_set_jpl_file
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1set_1jpl_1file(JNIEnv *env, jclass swephexp, jstring jplPath) {
    GET_STRING_UTF_CHARS(isCopy, jplPath, jplFile)
    //CPY_CSTRING_TO_CHARS(jplFile, fname) SINCE: 2.10.03

    swe_set_jpl_file(jplFile);

    RLZ_STRING_UTF_CHARS(isCopy, jplPath, jplFile)
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_get_planet_name
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_swisseph_SwephExp_swe_1get_1planet_1name(JNIEnv *env, jclass swephexp, jint ipl) {
    char s[AS_MAXCH];
    jstring result = NULL;
    const char *pname = swe_get_planet_name(ipl, s);
    if (NULL != pname) result = (*env)->NewStringUTF(env, pname);
    return result;
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_set_topo
 * Signature: (DDD)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1set_1topo(JNIEnv *env, jclass swephexp, jdouble geolon, jdouble geolat, jdouble geoalt) {
    swe_set_topo(geolon, geolat, geoalt);
}

/*
 * void swe_set_sid_mode(int32 sid_mode, double t0, double ayan_t0)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_set_sid_mode
 * Signature: (IDD)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1set_1sid_1mode(JNIEnv *env, jclass swephexp, jint sid_mode, jdouble t0, jdouble ayan_t0) {
    swe_set_sid_mode(sid_mode, t0, ayan_t0);
}

/*
 * int32 swe_get_ayanamsa_ex(double tjd_et, int32 iflag, double *daya, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_get_ayanamsa_ex
 * Signature: (DI[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1get_1ayanamsa_1ex(JNIEnv *env, jclass swephexp, jdouble tjd_et, jint iflag,
                                              jdoubleArray dayaArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy, dayaArray, daya)

    int32 retc = swe_get_ayanamsa_ex(tjd_et, iflag, daya, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy, dayaArray, daya)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/* int32 swe_get_ayanamsa_ex_ut(double tjd_ut, int32 iflag, double *daya, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_get_ayanamsa_ex_ut
 * Signature: (DI[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1get_1ayanamsa_1ex_1ut(JNIEnv *env, jclass swephexp, jdouble tjd_ut, jint iflag,
                                                  jdoubleArray dayaArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy, dayaArray, daya)

    int32 retc = swe_get_ayanamsa_ex_ut(tjd_ut, iflag, daya, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy, dayaArray, daya)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_get_ayanamsa
 * Signature: (D)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1get_1ayanamsa(JNIEnv *env, jclass swephexp, jdouble tjd_et) {
    return swe_get_ayanamsa(tjd_et);
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_get_ayanamsa_ut
 * Signature: (D)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1get_1ayanamsa_1ut(JNIEnv *env, jclass swephexp, jdouble tjd_ut) {
    return swe_get_ayanamsa_ut(tjd_ut);
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_get_ayanamsa_name
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_swisseph_SwephExp_swe_1get_1ayanamsa_1name(JNIEnv *env, jclass swephexp, jint isidmode) {
    jstring result = NULL;
    const char *name = swe_get_ayanamsa_name(isidmode);
    if (NULL != name) result = (*env)->NewStringUTF(env, name);
    return result;
}

/*
 * const char * swe_get_current_file_data(int ifno, double *tfstart, double *tfend, int *denum)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_get_current_file_data
 * Signature: (I[D[D[I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_swisseph_SwephExp_swe_1get_1current_1file_1data(JNIEnv *env, jclass swephexp, jint ifno,
                                                     jdoubleArray tfstartArray, jdoubleArray tfendArray,
                                                     jintArray denumArray) {
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, tfstartArray, tfstart)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, tfendArray, tfend)
    GET_INT_ARRAY_ELEMENTS(isCopy, denumArray, denum)

    int32 sweph_denum[1];
    jstring result = NULL;
    const char *data = swe_get_current_file_data(ifno, tfstart, tfend, &sweph_denum[0]);
    if (NULL != data) result = (*env)->NewStringUTF(env, data);
    denum[0] = sweph_denum[0];

    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy1, tfstartArray, tfstart)
    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy2, tfendArray, tfend)
    RLZ_INT_ARRAY_ELEMENTS_OK(isCopy, denumArray, denum)

    return result;
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_date_conversion
 * Signature: (IIIDC[D)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1date_1conversion(JNIEnv *env, jclass swephexp,
                                             jint y, jint m, jint d, jdouble utime,
                                             jchar calendarType, jdoubleArray tjdArray) {
    /* calendarType - g[regorian]|j[ulian] */
    /* utime - universal time in hours (decimal) */
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy, tjdArray, tjd)

    int retc = swe_date_conversion(y, m, d, utime, calendarType, tjd);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy, tjdArray, tjd)
    return retc;
}

/*
 * double swe_julday(int year, int month, int day, double hour, int gregflag)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_julday
 * Signature: (IIIDI)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1julday(JNIEnv *env, jclass swephexp,
                                   jint year, jint month, jint day, jdouble hour, jint gregflag) {
    return swe_julday(year, month, day, hour, gregflag);
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_revjul
 * Signature: (DI[I[D)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1revjul(JNIEnv *env, jclass swephexp, jdouble jd,
                                   jint gregflag, jintArray ymdArray, jdoubleArray utimeArray) {
    int ymd[3];
    GET_INT_ARRAY_ELEMENTS(isIntCopy, ymdArray, ymdOut)
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, utimeArray, utime)

    swe_revjul(jd, gregflag, &ymd[0], &ymd[1], &ymd[2], utime);

    ymdOut[0] = ymd[0];
    ymdOut[1] = ymd[1];
    ymdOut[2] = ymd[2];

    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isDblCopy, utimeArray, utime)
    RLZ_INT_ARRAY_ELEMENTS_OK(isIntCopy, ymdArray, ymdOut)
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_utc_to_jd
 * Signature: (IIIIIDI[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1utc_1to_1jd(JNIEnv *env, jclass swephexp, jint iyear, jint imonth,
                                        jint iday, jint ihour, jint imin, jdouble dsec,
                                        jint gregflag, jdoubleArray dretArray,
                                        jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy, dretArray, dret)

    int32 retc = swe_utc_to_jd(iyear, imonth, iday, ihour, imin, dsec, gregflag, dret, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy, dretArray, dret)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_jdet_to_utc
 * Signature: (DI[I[D)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1jdet_1to_1utc(JNIEnv *env, jclass swephexp,
                                          jdouble tjd_et, jint gregflag, jintArray ymdhmArray,
                                          jdoubleArray dsecArray) {
    GET_INT_ARRAY_ELEMENTS(isIntCopy, ymdhmArray, ymdhmOut)
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, dsecArray, dsecOut)

    int32 ymdhm[5];
    swe_jdet_to_utc(tjd_et, gregflag, &ymdhm[0], &ymdhm[1],
                    &ymdhm[2], &ymdhm[3], &ymdhm[4], dsecOut);

    ymdhmOut[0] = ymdhm[0];
    ymdhmOut[1] = ymdhm[1];
    ymdhmOut[2] = ymdhm[2];
    ymdhmOut[3] = ymdhm[3];
    ymdhmOut[4] = ymdhm[4];

    RLZ_INT_ARRAY_ELEMENTS_OK(isIntCopy, ymdhmArray, ymdhmOut)
    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isDblCopy, dsecArray, dsecOut)
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_jdut1_to_utc
 * Signature: (DI[I[D)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1jdut1_1to_1utc(JNIEnv *env, jclass swephexp,
                                           jdouble tjd_ut, jint gregflag, jintArray ymdhmArray,
                                           jdoubleArray dsecArray) {
    GET_INT_ARRAY_ELEMENTS(isIntCopy, ymdhmArray, ymdhmOut)
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, dsecArray, dsecOut)

    int32 ymdhm[5];
    swe_jdut1_to_utc(tjd_ut, gregflag, &ymdhm[0], &ymdhm[1],
                     &ymdhm[2], &ymdhm[3], &ymdhm[4], dsecOut);

    ymdhmOut[0] = ymdhm[0];
    ymdhmOut[1] = ymdhm[1];
    ymdhmOut[2] = ymdhm[2];
    ymdhmOut[3] = ymdhm[3];
    ymdhmOut[4] = ymdhm[4];

    RLZ_INT_ARRAY_ELEMENTS_OK(isIntCopy, ymdhmArray, ymdhmOut)
    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isDblCopy, dsecArray, dsecOut)
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_utc_time_zone
 * Signature: (IIIIIDD[I[D)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1utc_1time_1zone(JNIEnv *env, jclass swephexp, jint iyear, jint imonth,
                                            jint iday, jint ihour, jint imin, jdouble dsec, jdouble d_timezone,
                                            jintArray ymdhmArray, jdoubleArray dsecArray) {
    GET_INT_ARRAY_ELEMENTS(isIntCopy, ymdhmArray, ymdhmOut)
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, dsecArray, dsecOut)

    int32 ymdhm[5];
    swe_utc_time_zone(iyear, imonth, iday, ihour, imin, dsec, d_timezone, &ymdhm[0],
                      &ymdhm[1], &ymdhm[2], &ymdhm[3], &ymdhm[4], dsecOut);

    ymdhmOut[0] = ymdhm[0];
    ymdhmOut[1] = ymdhm[1];
    ymdhmOut[2] = ymdhm[2];
    ymdhmOut[3] = ymdhm[3];
    ymdhmOut[4] = ymdhm[4];

    RLZ_INT_ARRAY_ELEMENTS_OK(isIntCopy, ymdhmArray, ymdhmOut)
    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isDblCopy, dsecArray, dsecOut)
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_houses
 * Signature: (DDDI[D[D)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1houses(JNIEnv *env, jclass swephexp, jdouble tjd_ut,
                                   jdouble geolat, jdouble geolon, jint hsys,
                                   jdoubleArray cuspsArray, jdoubleArray ascmcArray) {
    GET_DOUBLE_ARRAY_ELEMENTS(isCuspsCopy, cuspsArray, cusps)
    GET_DOUBLE_ARRAY_ELEMENTS(isAscmcCopy, ascmcArray, ascmc)

    int retc = swe_houses(tjd_ut, geolat, geolon, hsys, cusps, ascmc);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCuspsCopy, cuspsArray, cusps)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isAscmcCopy, ascmcArray, ascmc)
    return retc;
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_houses_ex
 * Signature: (DIDDI[D[D)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1houses_1ex(JNIEnv *env, jclass swephexp, jdouble tjd_ut,
                                       jint iflag, jdouble geolat, jdouble geolon, jint hsys,
                                       jdoubleArray cuspsArray, jdoubleArray ascmcArray) {
    GET_DOUBLE_ARRAY_ELEMENTS(isCuspsCopy, cuspsArray, cusps)
    GET_DOUBLE_ARRAY_ELEMENTS(isAscmcCopy, ascmcArray, ascmc)

    int retc = swe_houses_ex(tjd_ut, iflag, geolat, geolon, hsys, cusps, ascmc);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCuspsCopy, cuspsArray, cusps)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isAscmcCopy, ascmcArray, ascmc)
    return retc;
}

/*
 * int swe_houses_ex2(double tjd_ut, int32 iflag, double geolat, double geolon, int hsys,
 *      double *cusps, double *ascmc, double *cusp_speed, double *ascmc_speed, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_houses_ex2
 * Signature: (DIDDI[D[D[D[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1houses_1ex2(JNIEnv *env, jclass swephexp,
                                        jdouble tjd_ut, jint iflag, jdouble geolat, jdouble geolon,
                                        jint hsys, jdoubleArray cuspsArray,
                                        jdoubleArray ascmcArray, jdoubleArray cuspSpeedArray,
                                        jdoubleArray ascmcSpeedArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, cuspsArray, cusps)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, ascmcArray, ascmc)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy3, cuspSpeedArray, cusp_speed)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy4, ascmcSpeedArray, ascmc_speed)

    int retc = swe_houses_ex2(tjd_ut, iflag, geolat, geolon, hsys, cusps, ascmc, cusp_speed, ascmc_speed, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy1, cuspsArray, cusps)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy2, ascmcArray, ascmc)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy3, cuspSpeedArray, cusp_speed)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy4, ascmcSpeedArray, ascmc_speed)
    ERR_BUILDER_APPEND_IF_SERR
    return retc;
}

/*
 * int swe_houses_armc(double armc,double geolat,double eps,int hsys,double *cusp,double *ascmc)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_houses_armc
 * Signature: (DDDI[D[D)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1houses_1armc(JNIEnv *env, jclass swephexp,
                                         jdouble armc, jdouble geolat, jdouble eps, jint hsys,
                                         jdoubleArray cuspArray, jdoubleArray ascmcArray) {
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, cuspArray, cusp)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, ascmcArray, ascmc)

    int retc = swe_houses_armc(armc, geolat, eps, hsys, cusp, ascmc);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy1, cuspArray, cusp)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy2, ascmcArray, ascmc)

    return retc;
}

/*
 * int swe_houses_armc_ex2(double armc, double geolat, double eps, int hsys, double *cusps,
 *      double *ascmc, double *cusp_speed, double *ascmc_speed, char *serr);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_houses_armc_ex2
 * Signature: (DDDI[D[D[D[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1houses_1armc_1ex2(JNIEnv *env, jclass swephexp, jdouble armc, jdouble geolat,
                                              jdouble eps, jint hsys, jdoubleArray cuspsArray,
                                              jdoubleArray ascmcArray, jdoubleArray cuspSpeedArray,
                                              jdoubleArray ascmcSpeedArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, cuspsArray, cusps)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, ascmcArray, ascmc)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy3, cuspSpeedArray, cusp_speed)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy4, ascmcSpeedArray, ascmc_speed)

    int retc = swe_houses_armc_ex2(armc, geolat, eps, hsys, cusps, ascmc, cusp_speed, ascmc_speed, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy1, cuspsArray, cusps)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy2, ascmcArray, ascmc)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy3, cuspSpeedArray, cusp_speed)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy4, ascmcSpeedArray, ascmc_speed)
    ERR_BUILDER_APPEND_IF_SERR
    return retc;
}

/*
 * double swe_house_pos(double armc, double geolat, double eps, int hsys, double *xpin, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_house_pos
 * Signature: (DDDI[DLjava/lang/StringBuilder;)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1house_1pos(JNIEnv *env, jclass swephexp, jdouble armc, jdouble geolat,
                                       jdouble eps, jint hsys, jdoubleArray xpinArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy, xpinArray, xpin)

    double house_pos = swe_house_pos(armc, geolat, eps, hsys, xpin, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy, xpinArray, xpin)
    ERR_BUILDER_APPEND_IF_SERR
    return house_pos;
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_house_name
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_swisseph_SwephExp_swe_1house_1name(JNIEnv *env, jclass swephexp, jint hsys) {
    jstring result = NULL;
    const char *house_name = swe_house_name(hsys);
    if (NULL != house_name) result = (*env)->NewStringUTF(env, house_name);
    return result;
}

/*
 * int32 swe_gauquelin_sector(double t_ut, int32 ipl, char *starname, int32 iflag, int32 imeth,
 *      double *geopos, double atpress, double attemp, double *dgsect, char *serr);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_gauquelin_sector
 * Signature: (DILjava/lang/StringBuilder;II[DDD[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1gauquelin_1sector(JNIEnv *env, jclass swephexp, jdouble t_ut, jint ipl,
                                              jobject starNameBuilder, jint iflag, jint imeth,
                                              jdoubleArray geoposArray, jdouble atpress, jdouble attemp,
                                              jdoubleArray dgsectArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isDgsectCopy, dgsectArray, dgsect)

    jstring starName = getBuilderString(env, starNameBuilder);
    GET_STRING_UTF_CHARS(isCopy, starName, starname)
    CPY_CSTRING_TO_CHARS(starname, star_name)

    int32 retc = swe_gauquelin_sector(t_ut, ipl, star_name, iflag, imeth, geopos, atpress, attemp, dgsect, serr);

    BUILDER_APPEND_IF_DIFF(starNameBuilder, starname, star_name)

    RLZ_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isDgsectCopy, dgsectArray, dgsect)
    RLZ_STRING_UTF_CHARS(isCopy, starName, starname)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_sol_eclipse_where(double tjd, int32 ifl, double *geopos, double *attr, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_sol_eclipse_where
 * Signature: (DI[D[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1sol_1eclipse_1where(JNIEnv *env, jclass swephexp, jdouble tjd, jint ifl,
                                                jdoubleArray geoposArray, jdoubleArray attrArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)

    int32 retc = swe_sol_eclipse_where(tjd, ifl, geopos, attr, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_lun_occult_where(double tjd, int32 ipl, char *starname, int32 ifl, double *geopos, double *attr, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_lun_occult_where
 * Signature: (DILjava/lang/StringBuilder;I[D[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1lun_1occult_1where(JNIEnv *env, jclass swephexp, jdouble tjd, jint ipl,
                                               jobject starNameBuilder, jint ifl, jdoubleArray geoposArray,
                                               jdoubleArray attrArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)

    jstring starName = getBuilderString(env, starNameBuilder);
    GET_STRING_UTF_CHARS(isCopy, starName, starname)
    CPY_CSTRING_TO_CHARS(starname, star_name)

    int32 retc = swe_lun_occult_where(tjd, ipl, star_name, ifl, geopos, attr, serr);

    BUILDER_APPEND_IF_DIFF(starNameBuilder, starname, star_name)

    RLZ_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)
    RLZ_STRING_UTF_CHARS(isCopy, starName, starname)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_sol_eclipse_how(double tjd, int32 ifl, double *geopos, double *attr, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_sol_eclipse_how
 * Signature: (DI[D[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1sol_1eclipse_1how(JNIEnv *env, jclass swephexp, jdouble tjd, jint ifl,
                                              jdoubleArray geoposArray, jdoubleArray attrArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)

    int32 retc = swe_sol_eclipse_how(tjd, ifl, geopos, attr, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_sol_eclipse_when_loc
 * Signature: (DI[D[D[DILjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1sol_1eclipse_1when_1loc(JNIEnv *env, jclass swephexp, jdouble tjd_start,
                                                    jint ifl, jdoubleArray geoposArray, jdoubleArray tretArray,
                                                    jdoubleArray attrArray, jint backward, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isTretCopy, tretArray, tret)
    GET_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)

    int32 retc = swe_sol_eclipse_when_loc(tjd_start, ifl, geopos, tret, attr, backward, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isTretCopy, tretArray, tret)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_lun_occult_when_loc(double tjd_start, int32 ipl, char *starname, int32 ifl,
 *      double *geopos, double *tret, double *attr, int32 backward, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_lun_occult_when_loc
 * Signature: (DILjava/lang/StringBuilder;I[D[D[DILjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1lun_1occult_1when_1loc(JNIEnv *env, jclass swephexp, jdouble tjd_start, jint ipl,
                                                   jobject starNameBuilder, jint ifl, jdoubleArray geoposArray,
                                                   jdoubleArray tretArray, jdoubleArray attrArray,
                                                   jint backward, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isTretCopy, tretArray, tret)
    GET_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)

    jstring starName = getBuilderString(env, starNameBuilder);
    GET_STRING_UTF_CHARS(isCopy, starName, starname)
    CPY_CSTRING_TO_CHARS(starname, star_name)

    int32 retc = swe_lun_occult_when_loc(tjd_start, ipl, star_name, ifl, geopos, tret, attr, backward, serr);

    BUILDER_APPEND_IF_DIFF(starNameBuilder, starname, star_name)

    RLZ_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isTretCopy, tretArray, tret)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)
    RLZ_STRING_UTF_CHARS(isCopy, starName, starname)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_sol_eclipse_when_glob
 * Signature: (DII[DILjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1sol_1eclipse_1when_1glob(JNIEnv *env, jclass swephexp,
                                                     jdouble tjd_start, jint ifl, jint ifltype,
                                                     jdoubleArray tretArray, jint backward,
                                                     jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy, tretArray, tret)

    int32 retc = swe_sol_eclipse_when_glob(tjd_start, ifl, ifltype, tret, backward, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy, tretArray, tret)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_lun_occult_when_glob(double tjd_start, int32 ipl, char *starname, int32 ifl,
 *      int32 ifltype,double *tret, int32 backward, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_lun_occult_when_glob
 * Signature: (DILjava/lang/StringBuilder;II[DILjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1lun_1occult_1when_1glob(JNIEnv *env, jclass swephexp, jdouble tjd_start, jint ipl,
                                                    jobject starNameBuilder, jint ifl, jint ifltype,
                                                    jdoubleArray tretArray, jint backward, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isTretCopy, tretArray, tret)

    jstring starName = getBuilderString(env, starNameBuilder);
    GET_STRING_UTF_CHARS(isCopy, starName, starname)
    CPY_CSTRING_TO_CHARS(starname, star_name)

    int32 retc = swe_lun_occult_when_glob(tjd_start, ipl, star_name, ifl, ifltype, tret, backward, serr);

    BUILDER_APPEND_IF_DIFF(starNameBuilder, starname, star_name)

    RLZ_DOUBLE_ARRAY_ELEMENTS(isTretCopy, tretArray, tret)
    RLZ_STRING_UTF_CHARS(isCopy, starName, starname)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_lun_eclipse_how(double tjd_ut,int32 ifl,double *geopos,double *attr,char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_lun_eclipse_how
 * Signature: (DI[D[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1lun_1eclipse_1how(JNIEnv *env, jclass swephexp, jdouble tjd_ut, jint ifl,
                                              jdoubleArray geoposArray, jdoubleArray attrArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)

    int32 retc = swe_lun_eclipse_how(tjd_ut, ifl, geopos, attr, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_lun_eclipse_when
 * Signature: (DII[DILjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1lun_1eclipse_1when(JNIEnv *env, jclass swephexp, jdouble tjd_start,
                                               jint ifl, jint ifltype, jdoubleArray tretArray,
                                               jint backward, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy, tretArray, tret)

    int32 retc = swe_lun_eclipse_when(tjd_start, ifl, ifltype, tret, backward, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy, tretArray, tret)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_lun_eclipse_when_loc
 * Signature: (DI[D[D[DILjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1lun_1eclipse_1when_1loc(JNIEnv *env, jclass swephexp, jdouble tjd_start,
                                                    jint ifl, jdoubleArray geoposArray, jdoubleArray tretArray,
                                                    jdoubleArray attrArray, jint backward, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isTretCopy, tretArray, tret)
    GET_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)

    int32 retc = swe_lun_eclipse_when_loc(tjd_start, ifl, geopos, tret, attr, backward, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isTretCopy, tretArray, tret)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isAttrCopy, attrArray, attr)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_pheno(double tjd, int32 ipl, int32 iflag, double *attr, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_pheno
 * Signature: (DII[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1pheno(JNIEnv *env, jclass swephexp, jdouble tjd, jint ipl,
                                  jint iflag, jdoubleArray attrArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy, attrArray, attr)

    int32 retc = swe_pheno(tjd, ipl, iflag, attr, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy, attrArray, attr)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_pheno_ut(double tjd_ut, int32 ipl, int32 iflag, double *attr, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_pheno_ut
 * Signature: (DII[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1pheno_1ut(JNIEnv *env, jclass swephexp, jdouble tjd_ut, jint ipl,
                                      jint iflag, jdoubleArray attrArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy, attrArray, attr)

    int32 retc = swe_pheno_ut(tjd_ut, ipl, iflag, attr, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy, attrArray, attr)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * double swe_refrac(double inalt, double atpress, double attemp, int calc_flag)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_refrac
 * Signature: (DDDI)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1refrac(JNIEnv *env, jclass swephexp, jdouble inalt,
                                   jdouble atpress, jdouble attemp, jint calc_flag) {
    return swe_refrac(inalt, atpress, attemp, calc_flag);
}

/*
 * double swe_refrac_extended(double inalt, double geoalt, double atpress,
 *      double attemp, double lapse_rate, int32 calc_flag, double *dret);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_refrac_extended
 * Signature: (DDDDDI[D)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1refrac_1extended(JNIEnv *env, jclass swephexp, jdouble inalt, jdouble geoalt,
                                             jdouble atpress, jdouble attemp, jdouble lapse_rate,
                                             jint calc_flag, jdoubleArray dretArray) {
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy, dretArray, dret)

    double rex = swe_refrac_extended(inalt, geoalt, atpress, attemp, lapse_rate, calc_flag, dret);

    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy, dretArray, dret)
    return rex;
}

/*
 * void swe_set_lapse_rate(double lapse_rate)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_set_lapse_rate
 * Signature: (D)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1set_1lapse_1rate(JNIEnv *env, jclass swephexp, jdouble lapse_rate) {
    swe_set_lapse_rate(lapse_rate);
}

/*
 * void swe_azalt(double tjd_ut,int32 calc_flag,double *geopos,double atpress,double attemp,double *xin,double *xaz)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_azalt
 * Signature: (DI[DDD[D[D)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1azalt(JNIEnv *env, jclass swephexp, jdouble tjd_ut, jint calc_flag,
                                  jdoubleArray geoposArray, jdouble atpress, jdouble attemp,
                                  jdoubleArray xinArray, jdoubleArray xazArray) {
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, xinArray, xin)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy3, xazArray, xaz)

    swe_azalt(tjd_ut, calc_flag, geopos, atpress, attemp, xin, xaz);

    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy1, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy2, xinArray, xin)
    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy3, xazArray, xaz)
}

/*
 * void swe_azalt_rev(double tjd_ut,int32 calc_flag,double *geopos,double *xin,double *xout);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_azalt_rev
 * Signature: (DI[D[D[D)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1azalt_1rev(JNIEnv *env, jclass swephexp, jdouble tjd_ut, jint calc_flag,
                                       jdoubleArray geoposArray, jdoubleArray xinArray, jdoubleArray xoutArray) {
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, xinArray, xin)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy3, xoutArray, xout)

    swe_azalt_rev(tjd_ut, calc_flag, geopos, xin, xout);

    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy1, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy2, xinArray, xin)
    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy3, xoutArray, xout)
}

/*
 * int32 swe_rise_trans_true_hor(double tjd_ut, int32 ipl, char *starname, int32 epheflag, int32 rsmi,
 *      double *geopos,double atpress, double attemp,double horhgt,double *tret,char *serr);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_rise_trans_true_hor
 * Signature: (DILjava/lang/StringBuilder;II[DDDD[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1rise_1trans_1true_1hor(JNIEnv *env, jclass swephexp, jdouble tjd_ut, jint ipl,
                                                   jobject starNameBuilder, jint epheflag, jint rsmi,
                                                   jdoubleArray geoposArray, jdouble atpress, jdouble attemp,
                                                   jdouble horhgt, jdoubleArray tretArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isTretCopy, tretArray, tret)

    jstring starName = getBuilderString(env, starNameBuilder);
    GET_STRING_UTF_CHARS(isCopy, starName, starname)
    CPY_CSTRING_TO_CHARS(starname, star_name)

    int32 retc = swe_rise_trans_true_hor(tjd_ut, ipl, star_name, epheflag,
                                         rsmi, geopos, atpress, attemp, horhgt, tret, serr);

    BUILDER_APPEND_IF_DIFF(starNameBuilder, starname, star_name)

    RLZ_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isTretCopy, tretArray, tret)
    RLZ_STRING_UTF_CHARS(isCopy, starName, starname)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_rise_trans(double tjd_ut, int32 ipl, char *starname, int32 epheflag, int32 rsmi,
 *      double *geopos, double atpress, double attemp, double *tret, char *serr);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_rise_trans
 * Signature: (DILjava/lang/StringBuilder;II[DDD[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1rise_1trans(JNIEnv *env, jclass swephexp, jdouble tjd_ut, jint ipl,
                                        jobject starNameBuilder, jint epheflag, jint rsmi,
                                        jdoubleArray geoposArray, jdouble atpress,
                                        jdouble attemp, jdoubleArray tretArray,
                                        jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    GET_DOUBLE_ARRAY_ELEMENTS(isTretCopy, tretArray, tret)

    jstring starName = getBuilderString(env, starNameBuilder);
    GET_STRING_UTF_CHARS(isCopy, starName, starname)
    CPY_CSTRING_TO_CHARS(starname, star_name)

    int32 retc = swe_rise_trans(tjd_ut, ipl, star_name, epheflag, rsmi, geopos, atpress, attemp, tret, serr);

    BUILDER_APPEND_IF_DIFF(starNameBuilder, starname, star_name)

    RLZ_DOUBLE_ARRAY_ELEMENTS(isGeoposCopy, geoposArray, geopos)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isTretCopy, tretArray, tret)
    RLZ_STRING_UTF_CHARS(isCopy, starName, starname)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_nod_aps(double tjd_et, int32 ipl, int32 iflag, int32 method,
 *      double *xnasc, double *xndsc, double *xperi, double *xaphe, char *serr);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_nod_aps
 * Signature: (DIII[D[D[D[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1nod_1aps(JNIEnv *env, jclass swephexp, jdouble tjd_et, jint ipl, jint iflag,
                                     jint method, jdoubleArray xnascArray, jdoubleArray xndscArray,
                                     jdoubleArray xperiArray, jdoubleArray xapheArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, xnascArray, xnasc)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, xndscArray, xndsc)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy3, xperiArray, xperi)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy4, xapheArray, xaphe)

    int32 retc = swe_nod_aps(tjd_et, ipl, iflag, method, xnasc, xndsc, xperi, xaphe, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy1, xnascArray, xnasc)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy2, xndscArray, xndsc)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy3, xperiArray, xperi)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy4, xapheArray, xaphe)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_nod_aps_ut(double tjd_ut, int32 ipl, int32 iflag, int32 method,
 *      double *xnasc, double *xndsc, double *xperi, double *xaphe, char *serr);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_nod_aps_ut
 * Signature: (DIII[D[D[D[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1nod_1aps_1ut(JNIEnv *env, jclass swephexp, jdouble tjd_ut, jint ipl, jint iflag,
                                         jint method, jdoubleArray xnascArray, jdoubleArray xndscArray,
                                         jdoubleArray xperiArray, jdoubleArray xapheArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, xnascArray, xnasc)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, xndscArray, xndsc)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy3, xperiArray, xperi)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy4, xapheArray, xaphe)

    int32 retc = swe_nod_aps_ut(tjd_ut, ipl, iflag, method, xnasc, xndsc, xperi, xaphe, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy1, xnascArray, xnasc)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy2, xndscArray, xndsc)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy3, xperiArray, xperi)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy4, xapheArray, xaphe)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_get_orbital_elements(double tjd_et, int32 ipl, int32 iflag, double *dret, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_get_orbital_elements
 * Signature: (DII[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1get_1orbital_1elements(JNIEnv *env, jclass swephexp, jdouble tjd_et, jint ipl,
                                                   jint iflag, jdoubleArray dretArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, dretArray, dret)

    int32 retc = swe_get_orbital_elements(tjd_et, ipl, iflag, dret, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isDblCopy, dretArray, dret)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_orbit_max_min_true_distance(double tjd_et, int32 ipl, int32 iflag,
 *      double *dmax, double *dmin, double *dtrue, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_orbit_max_min_true_distance
 * Signature: (DII[D[D[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1orbit_1max_1min_1true_1distance(JNIEnv *env, jclass swephexp, jdouble tjd_et,
                                                            jint ipl, jint iflag, jdoubleArray dmaxArray,
                                                            jdoubleArray dminArray, jdoubleArray dtrueArray,
                                                            jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, dmaxArray, dmax)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, dminArray, dmin)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy3, dtrueArray, dtrue)

    int32 retc = swe_orbit_max_min_true_distance(tjd_et, ipl, iflag, dmax, dmin, dtrue, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy1, dmaxArray, dmax)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy2, dminArray, dmin)
    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy3, dtrueArray, dtrue)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_deltat
 * Signature: (D)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1deltat(JNIEnv *env, jclass swephexp, jdouble tjd) {
    return swe_deltat(tjd);
}

/*
 * Class:     swisseph_SwephExp
 * Method:    swe_deltat_ex
 * Signature: (DILjava/lang/StringBuilder;)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1deltat_1ex(JNIEnv *env, jclass swephexp,
                                       jdouble tjd, jint iflag, jobject errBuilder) {
    DEFINE_CHAR_SERR

    double deltat = swe_deltat_ex(tjd, iflag, serr);

    ERR_BUILDER_APPEND_IF_SERR
    return deltat;
}

/*
 * int32 swe_time_equ(double tjd, double *te, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_time_equ
 * Signature: (D[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1time_1equ(JNIEnv *env, jclass swephexp, jdouble tjd,
                                      jdoubleArray teArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, teArray, te)

    int32 retc = swe_time_equ(tjd, te, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isDblCopy, teArray, te)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_lmt_to_lat(double tjd_lmt, double geolon, double *tjd_lat, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_lmt_to_lat
 * Signature: (DD[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1lmt_1to_1lat(JNIEnv *env, jclass swephexp, jdouble tjd_lmt, jdouble geolon,
                                         jdoubleArray tjdLatArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, tjdLatArray, tjd_lat)

    int32 retc = swe_lmt_to_lat(tjd_lmt, geolon, tjd_lat, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isDblCopy, tjdLatArray, tjd_lat)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * int32 swe_lat_to_lmt(double tjd_lat, double geolon, double *tjd_lmt, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_lat_to_lmt
 * Signature: (DD[DLjava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1lat_1to_1lmt(JNIEnv *env, jclass swephexp, jdouble tjd_lat, jdouble geolon,
                                         jdoubleArray tjdLmtArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, tjdLmtArray, tjd_lmt)

    int32 retc = swe_lat_to_lmt(tjd_lat, geolon, tjd_lmt, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isDblCopy, tjdLmtArray, tjd_lmt)
    ERR_BUILDER_APPEND_IF_ERR
    return retc;
}

/*
 * double swe_sidtime0(double tjd_ut, double eps, double nut)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_sidtime0
 * Signature: (DDD)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1sidtime0(JNIEnv *env, jclass swephexp, jdouble tjd_ut, jdouble eps, jdouble nut) {
    return swe_sidtime0(tjd_ut, eps, nut);
}

/*
 * double swe_sidtime(double tjd_ut)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_sidtime
 * Signature: (D)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1sidtime(JNIEnv *env, jclass swephexp, jdouble tjd_ut) {
    return swe_sidtime(tjd_ut);
}

/*
 * void swe_set_interpolate_nut(AS_BOOL do_interpolate)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_set_interpolate_nut
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1set_1interpolate_1nut(JNIEnv *env, jclass swephexp, jint do_interpolate) {
    swe_set_interpolate_nut(do_interpolate);
}

/*
 * void swe_cotrans(double *xpo, double *xpn, double eps)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_cotrans
 * Signature: ([D[DD)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1cotrans(JNIEnv *env, jclass swephexp,
                                    jdoubleArray xpoArray, jdoubleArray xpnArray, jdouble eps) {
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, xpoArray, xpo)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, xpnArray, xpn)

    swe_cotrans(xpo, xpn, eps);

    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy1, xpoArray, xpo)
    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy2, xpnArray, xpn)
}

/*
 * void swe_cotrans_sp(double *xpo, double *xpn, double eps)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_cotrans_sp
 * Signature: ([D[DD)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1cotrans_1sp(JNIEnv *env, jclass swephexp,
                                        jdoubleArray xpoArray, jdoubleArray xpnArray, jdouble eps) {
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, xpoArray, xpo)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, xpnArray, xpn)

    swe_cotrans_sp(xpo, xpn, eps);

    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy1, xpoArray, xpo)
    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy2, xpnArray, xpn)
}

/*
 * double swe_get_tid_acc(void)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_get_tid_acc
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1get_1tid_1acc(JNIEnv *env, jclass swephexp) {
    return swe_get_tid_acc();
}

/*
 * void swe_set_tid_acc(double t_acc)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_set_tid_acc
 * Signature: (D)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1set_1tid_1acc(JNIEnv *env, jclass swephexp, jdouble t_acc) {
    swe_set_tid_acc(t_acc);
}

/*
 * void swe_set_delta_t_userdef(double dt)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_set_delta_t_userdef
 * Signature: (D)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1set_1delta_1t_1userdef(JNIEnv *env, jclass swephexp, jdouble dt) {
    swe_set_delta_t_userdef(dt);
}

/*
 * double swe_degnorm(double x);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_degnorm
 * Signature: (D)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1degnorm(JNIEnv *env, jclass swephexp, jdouble x) {
    return swe_degnorm(x);
}

/*
 * double swe_radnorm(double x);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_radnorm
 * Signature: (D)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1radnorm(JNIEnv *env, jclass swephexp, jdouble x) {
    return swe_radnorm(x);
}

/*
 * double swe_rad_midp(double x1, double x0);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_rad_midp
 * Signature: (DD)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1rad_1midp(JNIEnv *env, jclass swephexp, jdouble x1, jdouble x0) {
    return swe_rad_midp(x1, x0);
}

/*
 * double swe_deg_midp(double x1, double x0);
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_deg_midp
 * Signature: (DD)D
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1deg_1midp(JNIEnv *env, jclass swephexp, jdouble x1, jdouble x0) {
    return swe_deg_midp(x1, x0);
}

/*
 * void swe_split_deg(double ddeg, int32 roundflag, int32 *ideg, int32 *imin, int32 *isec, double *dsecfr, int32 *isgn)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_split_deg
 * Signature: (DI[I[D[I)V
 */
JNIEXPORT void JNICALL
Java_swisseph_SwephExp_swe_1split_1deg(JNIEnv *env, jclass swephexp, jdouble ddeg, jint roundflag,
                                       jintArray iDmsArray, jdoubleArray dSfrArray, jintArray iSgnArray) {
    GET_INT_ARRAY_ELEMENTS(isInt1Copy, iDmsArray, dmsOut)
    GET_INT_ARRAY_ELEMENTS(isInt2Copy, iSgnArray, sgnOut)
    GET_DOUBLE_ARRAY_ELEMENTS(isDblCopy, dSfrArray, sfrOut)

    int32 dmss[4];
    double scfr[1];

    swe_split_deg(ddeg, roundflag, &dmss[0], &dmss[1], &dmss[2], &scfr[0], &dmss[3]);

    dmsOut[0] = dmss[0];
    dmsOut[1] = dmss[1];
    dmsOut[2] = dmss[2];
    sgnOut[0] = dmss[3];
    sfrOut[0] = scfr[0];

    RLZ_INT_ARRAY_ELEMENTS_OK(isInt1Copy, iDmsArray, dmsOut)
    RLZ_INT_ARRAY_ELEMENTS_OK(isInt2Copy, iSgnArray, sgnOut)
    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isDblCopy, dSfrArray, sfrOut)
}

/*
 * double swe_solcross(double x2cross, double jd_et, int32 flag, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_solcross
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1solcross(JNIEnv *env, jclass swephexp, jdouble x2cross, jdouble jd_et, jint flag, jobject errBuilder) {
    DEFINE_CHAR_SERR

    jdouble juldate = swe_solcross(x2cross, jd_et, flag, serr);

    ERR_BUILDER_APPEND_IF_SERR
    return juldate;
}

/*
 * double swe_solcross_ut(double x2cross, double jd_ut, int32 flag, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_solcross_ut
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1solcross_1ut(JNIEnv *env, jclass swephexp, jdouble x2cross, jdouble jd_ut, jint flag, jobject errBuilder) {
    DEFINE_CHAR_SERR

    jdouble juldate = swe_solcross_ut(x2cross, jd_ut, flag, serr);

    ERR_BUILDER_APPEND_IF_SERR
    return juldate;
}

/*
 * double swe_mooncross(double x2cross, double jd_et, int flag, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_mooncross
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1mooncross(JNIEnv *env, jclass swephexp, jdouble x2cross, jdouble jd_et, jint flag, jobject errBuilder) {
    DEFINE_CHAR_SERR

    jdouble juldate = swe_mooncross(x2cross, jd_et, flag, serr);

    ERR_BUILDER_APPEND_IF_SERR
    return juldate;
}

/*
 * double swe_mooncross_ut(double x2cross, double jd_ut, int flag, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_mooncross_ut
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1mooncross_1ut(JNIEnv *env, jclass swephexp, jdouble x2cross, jdouble jd_ut, jint flag, jobject errBuilder) {
    DEFINE_CHAR_SERR

    jdouble juldate = swe_mooncross_ut(x2cross, jd_ut, flag, serr);

    ERR_BUILDER_APPEND_IF_SERR
    return juldate;
}

/*
 * double swe_mooncross_node(double jd_et, int flag, double *xlon, double *xla, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_mooncross_node
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1mooncross_1node(JNIEnv *env, jclass swephexp, jdouble jd_et, jint flag,
                                            jdoubleArray xlonArray, jdoubleArray xlatArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, xlonArray, xlon)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, xlatArray, xlat)

    double juldate = swe_mooncross_node(jd_et, flag, xlon, xlat, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy1, xlonArray, xlon)
    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy2, xlatArray, xlat)
    ERR_BUILDER_APPEND_IF_SERR
    return juldate;
}

/*
 * double swe_mooncross_node_ut(double jd_ut, int flag, double *xlon, double *xla, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_mooncross_node_ut
 */
JNIEXPORT jdouble JNICALL
Java_swisseph_SwephExp_swe_1mooncross_1node_1ut(JNIEnv *env, jclass swephexp, jdouble jd_ut, jint flag,
                                                jdoubleArray xlonArray, jdoubleArray xlatArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, xlonArray, xlon)
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy2, xlatArray, xlat)

    double juldate = swe_mooncross_node_ut(jd_ut, flag, xlon, xlat, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy1, xlonArray, xlon)
    RLZ_DOUBLE_ARRAY_ELEMENTS_OK(isCopy2, xlatArray, xlat)
    ERR_BUILDER_APPEND_IF_SERR
    return juldate;
}

/*
 * int32 swe_helio_cross(int ipl, double x2cross, double jd_et, int iflag, int dir, double *jd_cross, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_helio_cross
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1helio_1cross(JNIEnv *env, jclass swephexp, jint ipl, jdouble x2cross, jdouble jd_et,
                                         jint iflag, jint dir, jdoubleArray jdCrossArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, jdCrossArray, jd_cross)

    int32 retc = swe_helio_cross(ipl, x2cross, jd_et, iflag, dir, jd_cross, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy1, jdCrossArray, jd_cross)
    ERR_BUILDER_APPEND_IF_SERR
    return retc;
}

/*
 * int32 swe_helio_cross_ut(int32 ipl, double x2cross, double jd_ut, int32 iflag, int32 dir, double *jd_cross, char *serr)
 *
 * Class:     swisseph_SwephExp
 * Method:    swe_helio_cross_ut
 */
JNIEXPORT jint JNICALL
Java_swisseph_SwephExp_swe_1helio_1cross_1ut(JNIEnv *env, jclass swephexp, jint ipl, jdouble x2cross, jdouble jd_ut,
                                         jint iflag, jint dir, jdoubleArray jdCrossArray, jobject errBuilder) {
    DEFINE_CHAR_SERR
    GET_DOUBLE_ARRAY_ELEMENTS(isCopy1, jdCrossArray, jd_cross)

    int32 retc = swe_helio_cross_ut(ipl, x2cross, jd_ut, iflag, dir, jd_cross, serr);

    RLZ_DOUBLE_ARRAY_ELEMENTS(isCopy1, jdCrossArray, jd_cross)
    ERR_BUILDER_APPEND_IF_SERR
    return retc;
}
