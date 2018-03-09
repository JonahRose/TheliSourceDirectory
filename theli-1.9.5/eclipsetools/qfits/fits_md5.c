/*----------------------------------------------------------------------------*/
/**
   @file	fits_md5.c
   @author	N. Devillard
   @date	May 2001
   @version	$Revision: 1.1 $
   @brief	FITS data block MD5 computation routine.

   This module offers MD5 computation over all data areas of a FITS file.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: fits_md5.c,v 1.1 2003/09/04 15:44:42 terben Exp $
	$Author: terben $
	$Date: 2003/09/04 15:44:42 $
	$Revision: 1.1 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "md5.h"
#include "fits_std.h"
#include "qerror.h"

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

/** Size of an MD5 hash in bytes (32 bytes are 128 bits) */
#define MD5HASHSZ	32

/*-----------------------------------------------------------------------------
  							Function code
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Compute the MD5 hash of data zones in a FITS file.
  @param	filename	Name of the FITS file to examine.
  @return	1 statically allocated character string, or NULL.

  This function expects the name of a FITS file.
  It will compute the MD5 hash on all data blocks in the main data section
  and possibly extensions (including zero-padding blocks if necessary) and
  return it as a string suitable for inclusion into a FITS keyword.

  The returned string is statically allocated inside this function,
  so do not free it or modify it. This function returns NULL in case
  of error.
 */
/*----------------------------------------------------------------------------*/
char * qfits_datamd5(char * filename)
{
	static char 		datamd5[MD5HASHSZ+1] ;
	struct MD5Context	ctx ;
	unsigned char 		digest[16] ;
	FILE     		*	in ;
	char 				buf[FITS_BLOCK_SIZE];
	char			*	buf_c ;
	int					i ;
	int					in_header ;
	int					check_fits ;

	/* Check entries */
	if (filename==NULL) return NULL ;
    /* Open input file */
    if ((in=fopen(filename, "r"))==NULL) {
        qfits_error("cannot open file %s", filename);
        return NULL ;
    }
    /* Initialize all variables */
    MD5Init(&ctx);
    in_header=1 ;
	check_fits=0 ;
    /* Loop over input file */
    while (fread(buf, 1, FITS_BLOCK_SIZE, in)==FITS_BLOCK_SIZE) {
		/* First time in the loop: check the file is FITS */
		if (check_fits==0) {
			/* Examine first characters in block */
			if (buf[0]!='S' ||
				buf[1]!='I' ||
				buf[2]!='M' ||
				buf[3]!='P' ||
				buf[4]!='L' ||
				buf[5]!='E' ||
				buf[6]!=' ' ||
				buf[7]!=' ' ||
				buf[8]!='=') {
				qfits_error("file [%s] is not FITS\n", filename);
				fclose(in);
				return NULL ;
			} else {
				check_fits=1 ;
			}
		}
        if (in_header) {
            buf_c = buf ;
            for (i=0 ; i<FITS_NCARDS ; i++) {
				if (buf_c[0]=='E' &&
					buf_c[1]=='N' &&
					buf_c[2]=='D' &&
					buf_c[3]==' ') {
                    in_header=0 ;
                    break ;
                }
                buf_c += FITS_LINESZ ;
            }
        } else {
            /* If current block is a data block */
			/* Try to locate an extension header */
			if (buf[0]=='X' &&
				buf[1]=='T' &&
				buf[2]=='E' &&
				buf[3]=='N' &&
				buf[4]=='S' &&
				buf[5]=='I' &&
				buf[6]=='O' &&
				buf[7]=='N' &&
				buf[8]=='=') {
                in_header=1 ;
				buf_c = buf ;
				for (i=0 ; i<FITS_NCARDS ; i++) {
					/* Try to find an END marker in this block */
					if (buf_c[0]=='E' &&
						buf_c[1]=='N' &&
						buf_c[2]=='D' &&
						buf_c[3]==' ') {
						/* Found END marker in same block as XTENSION */
						in_header=0;
						break ;
					}
					buf_c += FITS_LINESZ ;
				}
            } else {
                MD5Update(&ctx, (unsigned char *)buf, FITS_BLOCK_SIZE);
            }
        }
    }
    fclose(in);
	if (check_fits==0) {
		/* Never went through the read loop: file is not FITS */
		qfits_error("file [%s] is not FITS", filename);
		return NULL ;
	}
    /* Got to the end of file: summarize */
    MD5Final(digest, &ctx);
    /* Write digest into a string */
    sprintf(datamd5,
    "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
    digest[ 0],
    digest[ 1],
    digest[ 2],
    digest[ 3],
    digest[ 4],
    digest[ 5],
    digest[ 6],
    digest[ 7],
    digest[ 8],
    digest[ 9],
    digest[10],
    digest[11],
    digest[12],
    digest[13],
    digest[14],
    digest[15]);
	return datamd5 ;
}

/* vim: set ts=4 et sw=4 tw=75 */
