#!/bin/bash -xv

# ***************************************
# ***** INSTRUMENT splitting script *****
# *****      created by THELI       *****
# ***************************************

# splits OMEGACAM@VST images

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

  RA=`get_key ${FILE} "RA"`
  if [ "${RA}" = "" ]; then
      RA=0.001
  fi

  DEC=`get_key ${FILE} "DEC"`
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

  OBJECT=`get_key ${FILE} "OBJECT"`
  if [ "${OBJECT}" = "" ]; then
      OBJECT=UNKNOWN
  fi

  EQUINOX=`get_key ${FILE} "EQUINOX" | sed 's/[^0-9.]//g'`
  if [ "${EQUINOX}" = "" ]; then
      EQUINOX=2000.0
  fi

  MJD=`get_key ${FILE} "MJD-OBS"`
  if [ "${MJD}" = "" ]; then
      MJD=52000.0
  fi

  FILTER=`get_key ${FILE} "HIERARCH ESO INS FILT1 NAME" | ${P_CLEANSTRING}`
  if [ "${FILTER}" = "" ]; then
      FILTER=UNKNOWN
  fi
  FILTER=`echo ${FILTER} | sed 's/_SDSS//g'`
  FILTER=`echo ${FILTER} | sed 's/_JOHN//g'`

  LST=`get_key ${FILE} "LST"`
  if [ "${LST}" = "" ]; then
      LST=0.0
  fi

  AIRMASS=`get_key ${FILE} "AIRMASS"`
  if [ "${AIRMASS}" = "" ]; then
      AIRMASS=`get_key ${FILE} "HIERARCH ESO TEL AIRM START"`
      if [ "${AIRMASS}" = "" ]; then
	  AIRMASS=1.0
      fi
  fi

  GABODSID=`${P_NIGHTID} -t 16:00:00 -d 31/12/1998 -m ${MJD} |\
      ${P_GAWK} ' ($1 ~ /Days/) {print $6}' | ${P_GAWK} 'BEGIN{ FS="."} {print $1}'`

  # Preserve the CD matrix if present.
  # If this does not work, replace the 'get_wcs' function below
  # with the explicit code in 'bash.include', and then modify it
  # according to your needs.
  k=2
  cpx1=${REFPIXX[1]}
  cpx2=${REFPIXY[1]}
  cd11=${PIXSCX}
  cd12="0.0"
  cd21="0.0"
  cd22=${PIXSCY}
  while [ "${k}" -le "${NCHIPS}" ]
  do
    cpx1="${cpx1},${REFPIXX[${k}]}"
    cpx2="${cpx2},${REFPIXY[${k}]}"
    cd11="${cd11},${PIXSCX}"
    cd12="${cd12},0.0}"
    cd21="${cd21},0.0}"
    cd22="${cd22},${PIXSCY}"
    k=$(( $k + 1 ))
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
     -M11 "1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1" \
     -M12 "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0" \
     -M21 "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0" \
     -M22 "1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1" \
     ${FILE}

  k=1
  while [ "${k}" -le "${NCHIPS}" ]
  do
    tmp=`get_key ${FILE} "DATE-OBS"`
    ${P_REPLACEKEY} ${BASE}_${k}.fits "DATE-OBS= ${tmp}" DUMMY1
    ${P_REPLACEKEY} ${BASE}_${k}.fits "MJD-OBS = ${MJD}" DUMMY2
    mv ${BASE}_${k}.fits OMEGA.${FILTER}.${tmp}_${k}.fits
    k=$(( $k + 1 ))
  done

  mv ${FILE} ORIGINALS/${BASE}${SUFFIX}

done