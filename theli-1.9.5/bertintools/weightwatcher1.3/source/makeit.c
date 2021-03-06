/*
 				makeit.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	WeightWatcher
*
*	Author:		E.BERTIN, (IAP, Leiden observatory & ESO)
*
*	Contents:	main program.
*
*	Last modify:	25/09/2000
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"define.h"
#include	"globals.h"
#include	"fitscat.h"
#include	"field.h"
#include	"vector.h"

/******************************** makeit *************************************/
/*
Manage the whole stuff.
*/
void	makeit()

  {
   vecstruct	**vec, *vector;
   picstruct	**wfield, **ffield, *field, *owfield, *offield;
   PIXTYPE	*pix, *pixin, *owstrip,
		val, threshd,threshu;	
   FLAGTYPE	*flagin, *flag,
		*pofmask,*ofmask,*fmask2,
		nofmask, fmask,wmask,fval, maxbit;	
   int		i,j, width, height, padsize;
   char		*charpix, *ofstrip;
   short	*shortpix;
   int		*intpix, *contextbuf;
   size_t      	bowl, spoonful, stripsize, npix, cumspoon;

  useprefs();			/* update things accor. to prefs parameters */

  owfield = offield = NULL;	/* No output weights or flags is the default */
  maxbit = 0;
  bowl = 0;

/* Open vector images */
  if (prefs.nvec_name)
    QMALLOC(vec, vecstruct *, prefs.nvec_name)
  else
    vec = NULL;
  for (i=0; i<prefs.nvec_name; i++)
    {
    vector = vec[i] = newvec(prefs.vec_name[i]);
    vector->ofmask = prefs.vec_mask[i];
    if (vector->ofmask > maxbit)
      maxbit = vector->ofmask;
    }

/* Open weight images */
  if (prefs.nweight_name)
    QMALLOC(wfield, picstruct *, prefs.nweight_name)
  else
    wfield = NULL;
  for (i=0; i<prefs.nweight_name; i++)
    {
    wfield[i] = field = newfield(prefs.weight_name[i], WEIGHT_FIELD, NULL);
    if (i && (field->width!=width || field->height!=height))
      error(EXIT_FAILURE, "*Error*: incompatible image size for ",
	field->filename);
    else
      {
      width = field->width;
      height = field->height;
      }
    field->threshd = (PIXTYPE)prefs.weight_threshd[i];
    field->threshu = (PIXTYPE)prefs.weight_threshu[i];
    if (field->threshd>field->threshu)
      warning("Overlapping thresholds for ", field->rfilename);
    field->ofmask[0] = prefs.weight_mask[i];
    if (field->ofmask[0] > maxbit)
      maxbit = field->ofmask[0];
   }

/* Open bitmap (flag) images */
  if (prefs.nflag_name)
    QMALLOC(ffield, picstruct *, prefs.nflag_name)
  else
    ffield = NULL;
  pofmask = prefs.flag_ofmask;
  nofmask = prefs.nflag_ofmask;
  for (i=0; i<prefs.nflag_name; i++)
    {
    ffield[i] = field = newfield(prefs.flag_name[i], FLAG_FIELD, NULL);
    if (i && (field->width!=width || field->height!=height))
      error(EXIT_FAILURE, "*Error*: incompatible image size for ",
	field->filename);
    else
      {
      width = field->width;
      height = field->height;
      }
    field->wmask = wmask = prefs.flag_wmask[i];

/*-- Examine the FLAG masks */
    field->fmask = fmask = prefs.flag_fmask[i];
    field->nfmask = 0;
    ofmask = field->ofmask;
    fmask2 = field->fmask2;
    for (j=0; j<31;j++)
      if (fmask&(fval = 1<<j))
        {
        field->nfmask++;
        *(fmask2++) = fval;
        if (nofmask--)
          {
          if ((fval=*(ofmask++)=*(pofmask++)) > maxbit)
            maxbit = fval;
          }
        else
          error(EXIT_FAILURE, "*Error*: missing output flag mask for ",
		field->filename);
        }
    }

/* Prepare the WEIGHT output-image */
  if (*prefs.oweight_name)
    {
    owfield = newfield(prefs.oweight_name,WEIGHT_FIELD,wfield?*wfield:*ffield);
    stripsize = owfield->stripheight*owfield->width;
    owstrip = (PIXTYPE *)owfield->strip;
    bowl = owfield->npix;
    }

/* Prepare the FLAG output-image */
  if (*prefs.oflag_name)
    {
/*-- We first make a dummy copy of a FLAG input-image as a reference */
    QMALLOC(field, picstruct, 1);
    *field = ffield?**ffield:**wfield;
/*-- Select the output FLAG image # of bits per pixel */
    if (maxbit >= (1<<15))
      field->bitpix = BP_LONG;
    else if (maxbit >= (1<<7))
      field->bitpix = BP_SHORT;
    else
      field->bitpix = BP_BYTE;

    offield = newfield(prefs.oflag_name, FLAG_FIELD, field);
    free(field);
    stripsize = offield->stripheight*offield->width;
    if (offield->bitpix == BP_LONG)
      ofstrip = (char *)offield->strip;
    else
      QMALLOC(ofstrip, char, stripsize*offield->bytepix);
    bowl = offield->npix;
    }

/* Prepare the "context buffer" for polygon masking */
  if (vec)
    {QMALLOC(contextbuf, int, offield?offield->width:owfield->width);}
  else
    contextbuf = NULL;

/* Enter the main loop */
  NFPRINTF(OUTPUT, "Processing...");
  cumspoon = 0;
  spoonful = stripsize;
  for (; bowl; bowl -= spoonful)
    {
    if (spoonful>bowl)
      spoonful = bowl;

/*-- Init the WEIGHT and FLAG buffers */
    if (owfield)
      for (pix=owfield->strip,npix=spoonful;npix--;)
        *(pix++) = 1.0;
    if (offield)
      memset(offield->strip, 0, spoonful*sizeof(FLAGTYPE));

/*-- First step: extract info from input weight maps */
    for (i=0; i<prefs.nweight_name; i++)
      {
      field = wfield[i];
      readdata(field, (PIXTYPE *)field->strip, spoonful);
      if (owfield)
        {
        pixin = (PIXTYPE *)field->strip;
        pix = owfield->strip;
        threshd = field->threshd;
        threshu = field->threshu;
        for (npix = spoonful; npix--;)
          *(pix++) *= (((val=*(pixin++))>threshu || val<threshd)?
			0.0: (i?1.0:val));
        }

      if (offield && (fmask=field->ofmask[0]))
        {
        pixin = (PIXTYPE *)field->strip;
        flag = offield->strip;
        threshd = field->threshd;
        threshu = field->threshu;
        for (npix = spoonful; npix--; flag++)
          if ((val=*(pixin++))>threshu || val<threshd)
            *flag |= fmask;
        }
      }

/*-- Second step: extract info from input flag maps */
    for (i=0; i<prefs.nflag_name; i++)
      {
      field = ffield[i];
      readidata(field, (FLAGTYPE *)field->strip, spoonful);
      if (owfield && (wmask=field->wmask))
        {
        flagin = (FLAGTYPE *)field->strip;
        pix = owfield->strip;
        for (npix = spoonful; npix--; pix++)
          if (*(flagin++)&wmask)
            *pix = 0.0;
        }

      if (offield && (fmask=field->fmask))
        {
        flagin = (FLAGTYPE *)field->strip;
        flag = offield->strip;
        for (npix = spoonful; npix--; flag++)
          if ((fval=*(flagin++))&fmask)
            {
            ofmask = field->ofmask;
            fmask2 = field->fmask2;
            for(j=field->nfmask; j--; ofmask++)
              if (fval&*(fmask2++))
                *flag |= *ofmask;
            }
        }
      }

/*-- Third step: extract info from input polygon vectors */
    for (i=0; i<prefs.nvec_name; i++)
      {
      vector = vec[i];
      if (owfield)
        vec_to_map(vector, owfield, cumspoon, spoonful, contextbuf);

      if (offield && (fmask=vector->ofmask))
        vec_to_map(vector, offield, cumspoon, spoonful, contextbuf);
      }
    cumspoon += spoonful;

/*-- Write WEIGHT buffer */

    if (owfield)
      {
      if (bswapflag)
        swapbytes(owstrip, 4, spoonful);
      QFWRITE(owstrip, spoonful*sizeof(PIXTYPE), owfield->file,
	offield->rfilename);
      }

/*-- Convert the FLAG 32bits data to the output buffer (8,16 or 32 bits) */
    if (offield)
      {
      if (offield->bitpix!=BP_LONG)
        {
        flag = (FLAGTYPE *)offield->strip;
        switch(offield->bitpix)
          {
          case BP_BYTE:
            charpix = ofstrip;
            for (npix = spoonful; npix--;)
              *(charpix++) = (BYTE)*(flag++);
            break;
          case BP_SHORT:
            shortpix = (short *)ofstrip;
            for (npix = spoonful; npix--;)
              *(shortpix++) = (BYTE)*(flag++);
            break;
          case BP_LONG:
            break;
          default:
            error(EXIT_FAILURE, "*Internal Error*: ","this should not occur!");
            break;
          }
        }
/*---- Write FLAG buffer */
      if (bswapflag)
        swapbytes(ofstrip, offield->bytepix, spoonful);
      QFWRITE(ofstrip, spoonful*offield->bytepix, offield->file,
	offield->rfilename);
      }
    }

/* Pad the written FITS files */
  NFPRINTF(OUTPUT, "Closing files...");
  QCALLOC(charpix, char, FBSIZE);
  if (owfield)
    {
    padsize = (FBSIZE -((owfield->npix*sizeof(PIXTYPE))%FBSIZE)) % FBSIZE;
    if (padsize)
      QFWRITE(charpix, padsize, owfield->file, owfield->rfilename);
    }
  if (offield)
    {
    padsize = (FBSIZE -((offield->npix*offield->bytepix)%FBSIZE)) % FBSIZE;
    if (padsize)
      QFWRITE(charpix, padsize, offield->file, offield->rfilename);
    }
  free(charpix);

/* Free and close everything */
  for (i=0; i<prefs.nvec_name; i++)
    endvec(vec[i]);
  free(vec);
  free(contextbuf);

  for (i=0; i<prefs.nweight_name; i++)
    endfield(wfield[i]);
  free(wfield);

  for (i=0; i<prefs.nflag_name; i++)
    endfield(ffield[i]);
  free(ffield);

  if (owfield)
    endfield(owfield);

  if (offield)
    endfield(offield);

  return;
  }


