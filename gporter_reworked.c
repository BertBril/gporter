//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Dec 2021 [bert]: rework from gporter.c at https://github.com/sascha2000/gporter
// Added the POI name to '-r' output, and cleaned up a lot of stuff I didn't understand in the original code
// Note that I value Sascha's work greatly, it's just that I am used to working in a different mode

#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>

unsigned char file_content[]={0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x35,0x31,0x32,0x2d,0x30,0x38,0x32,0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x42,0x54,0x4a,0x00,0x7c,0x33,0x0e,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

#define mERR_BADFMT1 1
#define mERR_BADFMT2 2
#define mERR_FILENAME 4
#define mERR_CANTWRITE 8
#define mERR_TDOPT 16
#define mERR_SYMBOL 32
#define mERR_READ 64


static char* replace_in_string( const char *s, char ch, const char *repl )
{
    int count=0;
    const char* t;
    for( t=s; *t; t++ )
        count += (*t == ch);

    size_t rlen = strlen(repl);
    char *res = malloc(strlen(s) + (rlen-1)*count + 1);
    char *ptr = res;
    for( t=s; *t; t++ )
    {
        if( *t != ch )
            *ptr++ = *t;
	else
	{
            memcpy( ptr, repl, rlen );
            ptr += rlen;
        }
    }
    *ptr = 0;
    return res;
}

static unsigned int error=0;


static int decodeCoord( int argc, char* argv[], char* rescoordstr, int i )
{
    int t; char x[100];
    *rescoordstr = '\0';

    if ( strlen(argv[i])!=2 )
	strncpy(rescoordstr,&argv[i][2],8);
    else if (argc>(i+1))
	strncpy(rescoordstr,argv[i+1],8);

    strcpy( rescoordstr, replace_in_string(rescoordstr,',',".") );

    for ( t=0; t<strlen(rescoordstr); t++ )
	if ( rescoordstr[t]=='.' )
	    return 0;

    error = error|mERR_BADFMT1;
    return 1;
}


static void setFilename( int argc, char* argv[], char* name, int iarg )
{
    if ( strlen(argv[iarg])==2 )
    {
	if ( argc>(iarg+1) )
	{
	    if ( argv[iarg+1][0] != '-' )
		strncpy(name,argv[iarg+1],255);
	    else
		error=error|mERR_FILENAME;
	}
    }
    else
    {
	if ( argv[iarg][2] != '-' )
	    strncpy( name, &argv[iarg][2], 255 );
	else
	    error=error|mERR_FILENAME;
    }
}


static uint32_t convertString( char *coordstr )
{
    char x[1000];
    int i=0,t,m=0,r=0;
    uint32_t value=0;

    for (t=0;t<strlen(coordstr);t++)
    {
	m++;
	if ( i==1 )
	    r++;
	if ( coordstr[t] != '.' )
	    value = value*10 + (coordstr[t]-'0');
	if ( coordstr[t]=='.' )
	    { m=0; i=1; }
	if ( r==5 )
	    break;
    }

    if ( i==0 )
	{ error=error|mERR_BADFMT2; return -1; }
    for ( i=m; i<5; i++ )
	value *= 10;

    return value;
}


#define mIsFlagArg(val) ( argv[iarg][0] == '-' && argv[iarg][1] == val && !argv[iarg][2] )
#define mStringEqual(var,val) ( !strcmp(var,val) )


int main( int argc, char* argv[] )
{
  FILE* fp;
  int idx, iarg, t, error=0;
  int8_t max=-1, symb=0;

  int32_t Nk=0, Sk=0, Ek=0, Wk=0;
  int32_t Ns=0, Ss=0, Es=0, Ws=0;

  char N[100]; char S[100]; char E[100]; char W[100];
  char filename[300]="\0"; char filename2[255]="\0";
  char gdate[255]="0000\0"; char gtime[255]="0000\0";
  char symbol[255]="0\0";

  for ( idx=0; idx<100; idx++ )
      N[idx] = S[idx] = E[idx] = W[idx] = 0;

    // PARSE args
    for( iarg=0; iarg<argc; iarg++ )
    {
	if ( mIsFlagArg('N') )
	{
	    decodeCoord( argc, argv, N, iarg );
	    Nk = convertString( N );
	    Ns = 1;
	}
	else if ( mIsFlagArg('S') )
	{
	    decodeCoord( argc, argv, S, iarg );
	    Sk = convertString( S );
	    Sk = -Sk;
	    Ss = 1;
	}
	else if ( mIsFlagArg('E') )
	{
	    decodeCoord(argc,argv,E,iarg);
	    Ek=convertString(E);
	    Es=1;
	}

	if ( mIsFlagArg('W') )
	{
	    decodeCoord(argc,argv,W,iarg);
	    Wk=convertString(W);
	    Wk = -Wk;
	    Ws=1;
	}

	if ( mIsFlagArg('o') )
	{
	    setFilename(argc,argv,filename,iarg);

	    if ( !mStringEqual(filename,"auto") )
	    {
		if ( strlen(filename)>3 )
		    error=error|mERR_FILENAME;
		for ( t=0; t<strlen(filename); t++ )
		    if ( (filename[t]<48) || (filename[t]>57) )
			error=error|mERR_FILENAME;
		if ( error==0 )
		    { max = atoi(filename)-1; goto setmax; }
	    }
	    else // auto filename
	    {
		int run = 0;
		struct dirent* dirEntry = 0;

		DIR* dirHandle = opendir(".");
		if ( dirHandle )
		{
		    while ( (dirEntry = readdir(dirHandle)) )
		    {
			if ( strlen(dirEntry->d_name)==3 )
			{
			    if (atoi(dirEntry->d_name)>max)
				max=atoi(dirEntry->d_name);
			    run++;
			}
		    }
		    closedir(dirHandle);
		    setmax:
		    if (max>125)
			error=error|mERR_CANTWRITE;
		    else
		    {
			max++;
			strcpy( filename2, "" );
			if (max<10)
			    strcat( filename2,"00" );
			else if ( max<100 )
			    strcat(filename2,"0");

			sprintf( filename, "%s%d", filename2, max );
		    }
		}
	    }
	}

	if ( mIsFlagArg('d') )
	{
	    setFilename( argc, argv, gdate, iarg );
	    if ( strlen(gdate)!=4 )
		error=error|mERR_TDOPT;
	    else
		for (t=0;t<strlen(gdate);t++)
		    if ( (gdate[t]<48) || (gdate[t]>57) )
			error=error|mERR_TDOPT;
	}

	if ( mIsFlagArg('t') )
	{
	    setFilename( argc, argv, gtime, iarg );
	    if ( strlen(gtime)!=4 )
		error=error|mERR_TDOPT;
	    else
		for ( t=0; t<strlen(gtime); t++ )
		    if ( (gtime[t]<48) || (gtime[t]>57) )
			error=error|mERR_TDOPT;
	}

	if ( mIsFlagArg('s') )
	{
	    setFilename(argc,argv,symbol,iarg);

	    if ( mStringEqual(symbol,"star") )
		symb=0;
	    else if ( mStringEqual(symbol,"house") )
		symb=1;
	    else if ( mStringEqual(symbol,"flag") )
		symb=2;
	    else if ( mStringEqual(symbol,"car") )
		symb=3;
	    else if ( mStringEqual(symbol,"eat") )
		symb=4;
	    else if ( mStringEqual(symbol,"bus") )
		symb=5;
	    else if ( mStringEqual(symbol,"gas") )
		symb=6;
	    else if ( mStringEqual(symbol,"skyscraper") )
		symb=7;
	    else if ( mStringEqual(symbol,"plane") )
		symb=8;
	    else if ( strlen(symbol)==1 )
		symb=atoi(symbol);
	    else
		error = error|mERR_SYMBOL;

	    if ( symb > 8 )
		error = error|mERR_SYMBOL;
	}

	    // '-r': read coordinates from POI file and print them
	if ( mIsFlagArg('r') )
	{
	    setFilename( argc, argv, filename, iarg );

	    fp = fopen( filename, "r" );
	    if ( !fp )
		error = error|mERR_READ;
	    else
	    {
		int32_t N=0, E=0;
		int zzu;

		for ( zzu=0; zzu<128; zzu++ )
		    file_content[zzu]=0;
		fgets( file_content, 128, fp );
		fclose( fp );

		N = le32toh(*((int32_t*) &file_content[76]));
		E = le32toh(*((int32_t*) &file_content[80]));
		if ( N==0 )
		{
		    error=error|mERR_READ;
		    fprintf( stderr, "N==0" );
		}
		if ( E==0 )
		{
		    error=error|mERR_READ;
		    fprintf( stderr, "E==0" );
		}
		if ( error&mERR_READ )
		    goto wrapup_errors;

		file_content[21] = 0;
		printf( "%s ", file_content+12 );
		static const double fac = 0.00001;
		printf( "%s%g ", N<0 ? "S" : "N", N<0 ? -N*fac : N*fac );
		printf( "%s%g\n", E<0 ? "W" : "E", E<0 ? -E*fac : E*fac );
	    }
	    goto wrapup_errors;
	}
    }

    if ( Ns==1)
	*((int32_t*)&file_content[76]) = htole32(Nk);
    else if ( Ss==1)
	*((int32_t*)&file_content[76]) = htole32(Sk);
    if ( Es==1)
	*((int32_t*)&file_content[80]) = htole32(Ek);
    else if ( Ws==1)
	*((int32_t*)&file_content[80]) = htole32(Wk);
    if ( filename[0]=='\0' )
	error=error | 4;
    if ( (max>=0) && (max<126) )
	*((int8_t*) &file_content[4]) = max;
    if ( strlen(gdate)==4 )
	for ( t=0; t<4; t++ )
	    *((char*)&file_content[12+t]) = (char)gdate[t];
    if (strlen(gtime)==4)
	for (t=0;t<4;t++)
	    *((char*)&file_content[17+t]) = (char)gtime[t];

    *((int8_t*)&file_content[16]) = '-';
    *((int8_t*)&file_content[1]) = symb;

    if ( error==0 )
    {
	printf( "\nwriting filename %s \n \n", filename );
	fp = fopen( filename, "w" );
	for ( idx=0; idx<128; idx++ )
	    fputc( file_content[idx], fp );
	fclose( fp );
    }

wrapup_errors:

    if ( error&mERR_BADFMT1 )
	fprintf( stderr, "\n Error, Wrong Coordinate Format [1].\n" );
    if ( error&mERR_BADFMT2 )
	fprintf( stderr, "\n Error, Wrong Coordinate Format [2].\n" );
    if ( error&mERR_FILENAME )
	fprintf( stderr, "\n Wrong Filename or Filename missing, use -o to set the same.\n" );
    if ( error&mERR_CANTWRITE )
	fprintf( stderr, "\n Auto option cannot write file, because of number conflicts.\n" );
    if ( error&mERR_TDOPT )
	fprintf( stderr, "\n Option for time or date -t -d format wrong.\n" );
    if ( error&mERR_SYMBOL )
	fprintf( stderr, "\n Option for POI symbol -s wrong. 0-8 is allowed only.\n" );
    if ( error&mERR_READ )
	fprintf( stderr, "\n Error reading '%s' with -r.\n", filename );

    return error;
}
