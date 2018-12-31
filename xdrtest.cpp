/*************************************************************************
*
* File Name	 : XDRTEST.CPP
*
* Description: used for debugging XDR.CPP
*
* Exports	 : main()
*
* Imports	 : xdrmem_create <xdr.h>    xdr_int <xdr.h>
*			   xdr_string <xdr.h>		xdr_vector <xdr.h>
*			   xdr_union <xdr.h>		xdr_destroy <xdr.h>
*              strcpy <string.h>		printf <stdio.h>
*              xdrstdio_create <xdr.h>  fopen <stdio.h>
*			   fclose <stdio.h>
*
* copyright (c) J”rg Caumanns, 1993 [caumanns@cs.tu-berlin.de]
*
*      DISCLAIMER OF WARRANTIES.
*      The code is provided "AS IS", without warranty of any kind.
*
*************************************************************************/
#include <os2def.h>
#include <xdr.h>
#include <stdio.h>
#include <string.h>

/*
* needed for xdr_union
*/
enum disc_type	{ END = 0, ERROR, INTEGER, CHARACTER };
xdr_discrim filter_dir[] =	{
	{ ERROR,	(xdrproc_t)xdr_int		},
	{ INTEGER,	(xdrproc_t)xdr_int		},
	{ CHARACTER,(xdrproc_t)xdr_char		},
	{ END,		(xdrproc_t)NULL		}	};

/************************************************************************/

main(void)	{
	XDR xdrE;

#ifdef MEMSTREAM
	CHAR *pb = new CHAR[4096];

	/*
	* create XDR memory stream
	*/
	xdrmem_create(&xdrE, pb, 4096, XDR_ENCODE);
#else
	FILE *pf;
	if((pf = fopen("test", "wb")) == (FILE *)NULL)
		return 0;

	xdrstdio_create(&xdrE, pf, XDR_ENCODE);
#endif

	/*
	* encode some objects
	*/
	INT i = 9;  					// encode integer value
	xdr_int(&xdrE, &i);

	CHAR *str = new CHAR[32];		// encode string
	strcpy(str, "test string");
	xdr_string(&xdrE, &str, 256);

	INT aint[4];					// encode array of integers
	for(INT j = 0; j < 4; j++)
		aint[j] = 100+j;
	xdr_vector(&xdrE, (CHAR *)aint, 4, 4, (xdrproc_t)xdr_int);

	union	{                  		// encode discriminated union
		INT		error;
		INT		integer;
		CHAR	character;
		} test_union;
	INT disc;
	test_union.character = 'c';
	disc = CHARACTER;
	xdr_union(&xdrE, &disc, (CHAR *)&test_union, filter_dir, NULL);

	/*
	* destroy XDR stream handle
	*/
	xdr_destroy(&xdrE);

#ifndef MEMSTREAM
	fclose(pf);
#endif

	/*
	* invalidate variables
	*/
	i = 0;
	strcpy(str, "xxxxxxxxxxxxxxxxxx");
	for(j = 0; j < 4; j++)
		aint[j] = 0;
	test_union.integer = 0;

	/*
	* create XDR memory stream to read previously written data
	*/
	XDR xdrD;
#ifdef MEMSTREAM
	xdrmem_create(&xdrD, pb, 4096, XDR_DECODE);
#else
	if((pf = fopen("test", "rb")) == (FILE *)NULL)
		return 0;

	xdrstdio_create(&xdrD, pf, XDR_DECODE);
#endif

	xdr_int(&xdrD, &i);
	xdr_string(&xdrD, &str, 256);
	xdr_vector(&xdrD, (CHAR *)aint, 4, 4, (xdrproc_t)xdr_int);
	xdr_union(&xdrD, &disc, (CHAR *)&test_union, filter_dir, NULL);

	xdr_destroy(&xdrD);

#ifndef MEMSTREAM
	fclose(pf);
#endif

	/*
	* display results
	*/
	printf("i = %d\n", i);
	printf("str = %s\n", str);
	for(j = 0; j < 4; j++)
		printf("aint[%d] = %d\n", j, aint[j]);
	printf("test_union.character = %c\n", test_union.character);

	return 0;
	}


