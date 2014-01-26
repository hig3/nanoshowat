/*
  nanoshowat.c -- Show the newest attendance record in a table form.
  Time-stamp: "2014-01-26 Sun 14:03 JST hig"
*/
 
/* testing data
    c:\users\aUSERNUMBER\AppData\Roaming\PopClick\{Activity,Attendance}Record\yyyymmddnn\yyyymmddnn_{AnswerList,AttendanceList}.csv
 
    environment variables
    USERNAME=aUSERNUMBER
    userprofile=c:\users\aUSERNUMBER
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#define _WIN32_WINNT 0x0400
#include "windows.h"
#endif

/*#define _CRT_SECURE_NO_DEPRECATE*/
  

typedef int bool;
#define TRUE 1
#define FALSE 0
  
#define LINELENGTH 1000
#define MAXGAKUSEKI 400
#define NTERMINALS 300
#define PATHNAMELENGTH 500
  
 
typedef struct {
  bool ignore_anygakusekiList;
  bool use_nanogakusekiList;
  bool show_cumulative;
  bool verbose;
  char gakusekiListfilename[PATHNAMELENGTH];
  char csvfilename[PATHNAMELENGTH];
  char datedirname[PATHNAMELENGTH];
} config;
 
/* each clicker terminal's current status */
typedef struct {
  bool received;
  bool thistime;
  bool verified;
} status;
  
  
int parse_options(int argc, char **argv, config *c);
void show_options();

void print_info(config *c, char *CSVFileName);
void print_formatted(status *s);

bool isingakusekiList(char *element, int *list, int length);
bool isvalidgakuseki(char *number_received);
bool isvalidrealname(int terminal, char *realname);
bool isreceivedbefore(char *_number_received);
  
/* FORMAT of attendance list */
/*#define TEST_ATTENDANCELIST "2013112702"*/
#define HEADERLINES 6
#define MEMOSTR "memo"
  
/* put it in the current directory, or specify with -g/-n option */
#define GAKUSEKILIST "gakusekiList.txt"
  
/* global variables */
int gakuseki[MAXGAKUSEKI];
int ngakuseki;

char VERIFIEDCHAR[]="*";
char NEWCHAR[]= ".";
char BLANKCHAR[]=" ";
 
int main(int argc, char **argv){
    
  FILE *fpgakuseki, *fpattendance;
  char row[LINELENGTH];
  char numbersent[LINELENGTH];
  char realname[LINELENGTH];
  
  status s[NTERMINALS];
  int i;
  int t; /* terminal number */
  int st;
  
  char CSVFileName[PATHNAMELENGTH];
  
#ifdef _MSC_VER
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind;
  char DirToSearch[PATHNAMELENGTH];
#endif
  
  
  config c;
  c.ignore_anygakusekiList=FALSE;
  c.use_nanogakusekiList=FALSE;
  c.show_cumulative=TRUE;
  c.verbose=FALSE;

  strncpy(c.gakusekiListfilename,GAKUSEKILIST,PATHNAMELENGTH-1);
  strcpy(c.csvfilename,""); /* '\0' terminated */
  strcpy(c.datedirname,""); /* '\0' terminated */

  parse_options(argc,argv,&c);
  
  if( !c.ignore_anygakusekiList && !c.use_nanogakusekiList ){
    if( (fpgakuseki=fopen(c.gakusekiListfilename,"r"))==NULL){
      fprintf(stderr,"Cannot open Gakuseki List: %s.\n",c.gakusekiListfilename);
      exit(0);
    }
    ngakuseki=0;
    while(fgets(row,sizeof(row),fpgakuseki)!=NULL){
#if 0
      sscanf_s(row,"%[0123456789]",numbersent,LINELENGTH);
#else
      sscanf(row,"%7[0123456789]",numbersent);
#endif
      gakuseki[ngakuseki]=atoi(numbersent);
      ngakuseki++;
    }
    fclose(fpgakuseki);
  }
  
  if( strcmp(c.csvfilename,"")==0 ){
    strncpy(CSVFileName,getenv("USERPROFILE"),PATHNAMELENGTH-1);
    strncat(CSVFileName,"\\AppData\\Roaming\\PopClick\\AttendanceRecord\\",PATHNAMELENGTH-strlen(CSVFileName)-1);
  
    if( strcmp(c.datedirname,"")==0 ){
#ifdef _MSC_VER


      strncpy(DirToSearch,CSVFileName,PATHNAMELENGTH-strlen(DirToSearch)-1);
      if(PATHNAMELENGTH-strlen(DirToSearch)-1>0){
	strncat(DirToSearch,"*",PATHNAMELENGTH-strlen(DirToSearch)-1);
      } else {
	fprintf(stderr,"Too long directory name %s\n",DirToSearch);
      }

      if(c.verbose){
	fprintf (stderr,"Target file is %s\n", DirToSearch);
      }
      hFind = FindFirstFile(DirToSearch, &FindFileData);
      if(c.verbose){
	fprintf (stderr,"The first file found is %s\n", FindFileData.cFileName);
      }
      while(FindNextFile(hFind, &FindFileData)){
	if (hFind == INVALID_HANDLE_VALUE) {
	  fprintf (stderr,"Invalid File Handle. Get Last Error reports %d\n", GetLastError ());
	  break;
	} else {
	  fprintf (stderr,"Next AttendanceData Directory %s.\n", FindFileData.cFileName);
	}
 
	if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
	  strncpy(c.datedirname,FindFileData.cFileName,PATHNAMELENGTH-strlen(c.datedirname)-1);
	}
      }
      FindClose(hFind);
#endif
    }

    strncat(CSVFileName,c.datedirname,PATHNAMELENGTH-strlen(CSVFileName)-1);
    strncat(CSVFileName,"\\",PATHNAMELENGTH-strlen(CSVFileName)-1);
    strncat(CSVFileName,c.datedirname,PATHNAMELENGTH-strlen(CSVFileName)-1);
    if(PATHNAMELENGTH-strlen(CSVFileName)-1<strlen("_AttendanceList.csv")){
      fprintf(stderr,"Too long filename: %s\n",CSVFileName);
      exit(0);
    } else {
      strncat(CSVFileName,"_AttendanceList.csv",PATHNAMELENGTH-strlen(CSVFileName)-1);
    }
  } else {
    strncpy(CSVFileName,c.csvfilename,PATHNAMELENGTH-1);
  }
  if( (fpattendance=fopen(CSVFileName,"r"))==NULL){
    fprintf(stderr,"Cannot open %s.\n",CSVFileName);
    exit(0);
  } else {
    if(c.verbose){
      fprintf(stderr,"Inspect %s\n",CSVFileName);
    }
  }
  
  for(i=0;i<NTERMINALS;i++){
    s[i].received=FALSE;
    s[i].verified=FALSE;
    s[i].thistime=FALSE;
  }
  
  i=0;
  while(fgets(row,sizeof(row),fpattendance)!=NULL){
    /* skip header lines */
    if(i<HEADERLINES){
      i++;
      continue;
    }
    strcpy(realname,"");
    strcpy(numbersent,"");
 
    /* restriction:  additionalinfo2 should be empty */
    /* number,realname,terminalid,additionalinfo1,additionalinfo2,attendance */
#if 0
    sscanf_s(row,"%d,%[^,],%*[^,],%[^,],,%d,%*s\n",&t,realname,LINELENGTH,numbersent,LINELENGTH,&st);
    /*    sscanf(row,"%d,%*[^,],%*[^,],%[^,],%*[^,],%d,%*s\n",&terminal,numbersent,&st); *//* %*[^,] does not match empty string. */
#else
    sscanf(row,"%d,%400[^,],%*[^,],%8[^,],,%d,%*s\n",&t,realname,numbersent,&st);
#endif
    if( c.verbose && (st!=0 || (c.show_cumulative && strstr(numbersent,MEMOSTR)!=numbersent))){
      printf("%d,%s,%s,%d\n",t,realname,numbersent,st);
    }
 
    if( c.ignore_anygakusekiList ){
      s[t].verified=isvalidgakuseki(numbersent);
    } else if ( c.use_nanogakusekiList ){
      s[t].verified=isvalidrealname(t,realname);
    } else {
      s[t].verified=isingakusekiList(numbersent,gakuseki,ngakuseki);
    }
 
    if(c.show_cumulative){
      s[t].received=isreceivedbefore(numbersent);
    } else {
      s[t].received=st;
    }
 
    if(st){
      s[t].thistime=TRUE;
      s[t].received=TRUE;
    }
 
  }
 
  fclose(fpattendance);
  
  print_info(&c, CSVFileName);
  print_formatted(s);
  
  return 0;
}
  
/* Linear search. One may use bisection method for speed up.
   if additional field 1 appear in the gakuseki list or not */
int isingakusekiList(char *element, int *list, int length){
  int r;
  int i;
  
  if(strlen(element)==0){
    return FALSE;
  }
 
  if(strstr(element,MEMOSTR)==element){
    return FALSE;
  }
 
  r=atoi(element);
  for(i=0;i<ngakuseki;i++){
    if( r==gakuseki[i] ){
      return TRUE;
    } else if( r<gakuseki[i] ){
      break;
    }
  }
  return FALSE;
}
  
/* if additional field 1 is valid or not */
bool isvalidgakuseki(char *received){
  int GAKUSEKIMIN=1000000;
  int GAKUSEKIMAX=9999999;
 
  if(strlen(received)==0){
    return FALSE;
  }
 
  if(strstr(received,MEMOSTR)==received){
    return FALSE;
  }
 
  if(atoi(received)>=GAKUSEKIMIN && atoi(received)<= GAKUSEKIMAX ){
    return TRUE;
  } else {
    return FALSE;
  }
 
}
 
bool isvalidrealname(int terminal, char *realname){
  if( terminal==atoi(realname) ){
    return FALSE;
  } else {
    return TRUE;
  }
}
 
 
bool isreceivedbefore(char *number_received){
  if(strstr(number_received,MEMOSTR)==number_received){
    return FALSE;
  } else {
    return TRUE;
  }
}
  
int parse_options(int argc, char **argv, config *c){
  int num_op=0;
  
  while ( argc > 1){
    if( argv[1][0]=='-' ){
      num_op++;
      switch( argv[1][1] ){
        
      case 'i':         /* use preregistered gakuseki list */
	c->ignore_anygakusekiList=TRUE;
	break;
  
      case 's':         /* show separate attendance */
	c->show_cumulative=FALSE;
	break;

      case 'm':
	strncpy(c->gakusekiListfilename,getenv("HOMEDRIVE"),PATHNAMELENGTH-1);
	strncat(c->gakusekiListfilename,getenv("HOMEPATH"),PATHNAMELENGTH-strlen(c->gakusekiListfilename)-1);
  strncat(c->gakusekiListfilename,"\\",PATHNAMELENGTH-strlen(c->gakusekiListfilename)-1);
  strncat(c->gakusekiListfilename,GAKUSEKILIST,PATHNAMELENGTH-strlen(c->gakusekiListfilename)-1);
	break;

      case 'c':         /* show separate attendance */
	strncpy(c->gakusekiListfilename,GAKUSEKILIST,PATHNAMELENGTH-1);
	break;

      case 'g':
	if( argc > 2 ){
	  if(strlen(argv[2])>PATHNAMELENGTH-1){
	    fprintf(stderr,"Too long gakusekiList.txt filename given: %s\n",argv[2]);
	    exit(0);
	  }
	  strncpy(c->gakusekiListfilename,argv[2],PATHNAMELENGTH-1);
	  argv++; argc--;
	} else {
	  fprintf(stderr,"No filename follows -%c.\n",argv[1][1]);
	}
	break;
 
      case 'n':
	c->use_nanogakusekiList=TRUE;
	break;

      case 'h':
	show_options();
	exit(0);
	break;
 
      case 'v':
	c->verbose=TRUE;
	break;
  
      case 'f':
	if( argc > 2 ){
	  if(strlen(argv[2])>PATHNAMELENGTH-1){
	    fprintf(stderr,"Too long CSV filename given: %s\n",argv[2]);
	    exit(0);
	  }
	  strncpy(c->csvfilename,argv[2],PATHNAMELENGTH-1);
	  argv++; argc--;
	} else {
	  fprintf(stderr,"No filename follows -%c.\n",argv[1][1]);
	}
	break;
 
      case 'd':
	if( argc > 2 ){
	  if(strlen(argv[2])>PATHNAMELENGTH-1){
	    fprintf(stderr,"Too long Attandance Directory name given: %s\n",argv[2]);
	    exit(0);
	  }
	  strncpy(c->datedirname,argv[2],PATHNAMELENGTH-1);
	  argv++; argc--;
	} else {
	  fprintf(stderr,"No directory name follows -%c.\n",argv[1][1]);
	}
	break;
 
      default:  /* óùâÇ≈Ç´Ç»Ç¢ option Ç™ó^Ç¶ÇÁÇÍÇΩ */
	fprintf(stderr,"Invalid option: -%c\n", argv[1][1]);
	show_options();
	num_op--;
      }
    }
    argv++; argc--;
  }
  
  return num_op;
}

void show_options(){
  fprintf(stderr,"-h show this help\n");
  fprintf(stderr,"-v :verbose mode\n");
  fprintf(stderr,"-c use gakusekiList.txt in the current directory\n");
  fprintf(stderr,"-g <gakusekiListFilename>\n");
  fprintf(stderr,"-m use gakusekiList.txt in my home directory, like Nano\n");
  fprintf(stderr,"-n use nano's built-in gakuseki list\n");
  fprintf(stderr,"-i ignore gakuseki list\n");
  fprintf(stderr,"-s show only the last attendance check result\n");
  fprintf(stderr,"-d <DirectoryName>\n");
  fprintf(stderr,"-f <CSVFileName>\n");
  return;
}
  
  
void print_info(config *c, char *CSVFilename){
  fprintf(stderr,"AttendanceList:%s\n",CSVFilename);
  if(c->ignore_anygakusekiList){
    fprintf(stderr,"gakusekiList:none\n");
  } else if(c->use_nanogakusekiList){
    fprintf(stderr,"gakusekiList: Taken from Clicker Nano\n");
  } else {
    fprintf(stderr,"gakusekiList:%s\n",c->gakusekiListfilename);
  }

  if(c->show_cumulative){  
    fprintf(stderr,"Range: All today's attendance check\n");
  } else {
    fprintf(stderr,"Range: Only the latest attendance check\n");
  }


  fprintf(stderr,"Latest: %s\nGakuseki verified: %s\n",NEWCHAR,VERIFIEDCHAR);
  fprintf(stderr,"\n");
  return;
}
  
void print_formatted(status *s){
  int NCOLS=10; /* number of columns of the table*/
  int i; /* col */
  int j; /* row */
  int k;

  int t;	/* terminal id */
  int mint=NTERMINALS; 	/* min id of terminal which responded */
  int maxt=1;
   
  int length;
  
  for(t=1;t<=NTERMINALS;t++){
    if( (s+t)->received ){
      mint=t;
      break;
    }
  }
  for(t=NTERMINALS;t>=1;t--){
    if( (s+t)->received ){
      maxt=t;
      break;
    }
  }
  
  /*  fprintf(stderr,"%d:%d\n",mint,maxt); */
  
  t=mint/NCOLS*NCOLS;
  for(j=mint/NCOLS;j<=(maxt-1)/NCOLS;j++){
    for(i=0;i<NCOLS;i++){
      t++;
      if(!(s+t)->received){
	printf("      ");
      } else {
	printf(" %3d",t);
 
	length=0;
	if(((s+t)->verified)){
	  printf("%s",VERIFIEDCHAR);
	  length++;
	}
 
	if(((s+t)->thistime)){
	  printf("%s",NEWCHAR);
	  length++;
	}
	for(k=0;k<2-length;k++){
	  printf("%s",BLANKCHAR);
	}
      }
    }
    printf("\n");
  }
}
