BASHPATH -xv

# ***************************************
# ***** INSTRUMENT splitting script *****
# *****      created by THELI       *****
# ***************************************

# splits ACS@HST images

# $1: main directory
# $2: science directory

INSTDIR=instruments_professional
if [ ! -f ${INSTDIR}/${INSTRUMENT}.ini ]; then
    INSTDIR=instruments_commercial
fi
if [ ! -f ${INSTDIR}/${INSTRUMENT}.ini ]; then
    INSTDIR=~/.theli/instruments_user/
fi
. ${INSTDIR}/${INSTRUMENT:?}.ini

. bash.include


# rename the images to the value of a FITS key (if requested)

if [ "${V_PRE_RENAME_CHECKED}" = "1" ]; then

  cd /$1/$2
  mkdir ORIGINALS


  p_ls > ${TEMPDIR}/fitskey_$$

  cat ${TEMPDIR}/fitskey_$$ |\
  {
    while read FILE
    do
      newname=`get_key ${FILE} ${V_RENAME_FITSKEY}`
      if [ "${newname}" != "" ]; then
         mv ${FILE} ${newname}
      else
         echo ERROR: Could not find ${V_RENAME_FITSKEY} in the FITS header
         mv ${FILE} ORIGINALS/
      fi
    done
  }
fi


cd /$1/$2

FILES=`p_ls`

mkdir ORIGINALS

for FILE in ${FILES}
do
  SUFFIX=`echo $FILE | ${P_GAWK} 'BEGIN {FS="."} {print "."$NF}'`
  BASE=`basename $FILE ${SUFFIX}`
  mv ${FILE} ${BASE}.fits
  FILE=${BASE}.fits

  RA=`get_key ${FILE} "CRVAL1"`
  if [ "${RA}" = "" ]; then
      RA=0.001
  fi

  DEC=`get_key ${FILE} "CRVAL2"`
  if [ "${DEC}" = "" ]; then
      DEC=0.0
  fi

  EXPTIME=`get_key ${FILE} "EXPTIME"`
  if [ "${EXPTIME}" = "" ]; then
      EXPTIME=`get_key ${FILE} "EXPOSURE"`
  fi
  if [ "${EXPTIME}" = "" ]; then
      EXPTIME=1.0
  fi

  OBJECT=`get_key ${FILE} "TARGNAME"`
  if [ "${OBJECT}" = "" ]; then
      OBJECT=UNKNOWN
  fi

  EQUINOX=`get_key ${FILE} "EQUINOX" | sed 's/[^0-9.]//g'`
  if [ "${EQUINOX}" = "" ]; then
      EQUINOX=2000.0
  fi

  MJD=`get_key ${FILE} "EXPSTART"`
  if [ "${MJD}" = "" ]; then
      MJD=52000.0
  fi

  FILTER=`get_key ${FILE} "FILTER1" | ${P_CLEANSTRING}`
  if [ "${FILTER}" = "" ]; then
      FILTER=UNKNOWN
  fi

  LST=0.0

  AIRMASS=1.0

  GABODSID=`${P_NIGHTID} -t 16:00:00 -d 31/12/1998 -m ${MJD} |\
      ${P_GAWK} ' ($1 ~ /Days/) {print $6}' | ${P_GAWK} 'BEGIN{ FS="."} {print $1}'`

  # Preserve the CD matrix if present.
  # If this does not work, replace the 'get_wcs' function below
  # with the explicit code in 'bash.include', and then modify it
  # according to your needs.
  get_wcs

  # Have to remove some offending COMMENT/CONTINUE keywords
  i=1000
  while [ ${i} -lt 2000 ]
  do
      ${P_REPLACEKEY} ${FILE} "D${i}   = 0" COMMENT >& ${TEMPDIR}/test_$$
      wc=`wc ${TEMPDIR}/test_$$ | ${P_GAWK} '{print $1}'`
      if [ ${wc} -gt 0 ]; then
	  break
      fi
      i=$((${i}+1))
  done
  while [ ${i} -lt 2000 ]
  do
      ${P_REPLACEKEY} ${FILE} "D${i}   = 0" CONTINUE >& ${TEMPDIR}/test_$$
      wc=`wc ${TEMPDIR}/test_$$ | ${P_GAWK} '{print $1}'`
      if [ ${wc} -gt 0 ]; then
	  break
      fi
      i=$((${i}+1))
  done

  ${P_FITSSPLIT_ECL} \
     -CRPIX1 "${cpx1}" \
     -CRPIX2 "${cpx2}" \
     -CD11 "${cd11}" \
     -CD12 "${cd12}" \
     -CD21 "${cd21}" \
     -CD22 "${cd22}" \
     -CRVAL1 ${RA} -CRVAL2 ${DEC} -EXPTIME ${EXPTIME} -AIRMASS ${AIRMASS} \
     -GABODSID ${GABODSID} -FILTER ${FILTER} -EQUINOX ${EQUINOX} -OBJECT ${OBJECT} \
     -M11 "1" \
     -M12 "0" \
     -M21 "0" \
     -M22 "1" \
     ${FILE}

  k=1
  while [ "${k}" -le "${NCHIPS}" ]
  do
    tmp=`get_key ${FILE} "MJD"`
    ${P_REPLACEKEY} ${BASE}_${k}.fits "MJD-OBS = ${tmp}" DUMMY2
    k=$(( $k + 1 ))
  done

  mv ${FILE} ORIGINALS/${BASE}${SUFFIX}

done
