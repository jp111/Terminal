/*
 * Jaspreet Singh 201202078
 * My Terminal
 *
/

/* ........................................................................ */
/* Defining the structure to store the details of all background processeds */

#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<signal.h>
#include<termios.h>
struct termios new, old;
int in,out,newPipe[2],oldPipe[2];
char filein[1000],fileout[1000];
char *str;
typedef struct node
{
char name[100][1000];
int p_id;
int status_value;
}node;
node ar[1000];//Defining array of structures
int stck=0,bck=0;//Defining stack pointer and background process flag
pid_t pid;
char presdir[1000],str3[1000];//to store present directory name and the path
/* ......................................... */
/* Print the prompt with string manipulation */
void doit()
{
char cwd[1000],username[200];
getcwd(cwd,1000);
gethostname(username,200);
printf("<%s@%s:",getlogin(),username);
if(strstr(cwd,presdir)==NULL)
{
printf("%s",cwd);
}
else
{
int k,i,len,j=0,flag=2,len1;
printf("~");
len=strlen(cwd);
len1=strlen(presdir);
//Finding the position of original directory in present working directory
for(i=0;i<len;i++)
{
j=0;
if(cwd[i]==presdir[j])
{
flag=0;
for(k=i;k<len;k++)
{
if(cwd[k]!=presdir[j] || j>=len1)
{
flag=1;
break;
}
j++;
}
if(flag==0)
break;
}
}
//Printing the rest of the part after ~
for(i=k;i<len;i++)
printf("%c",cwd[i]);
}
printf(">");
}
//Handling the cd command along with errors
void doitcd(char *str1[1000])
{
int len,i=0;
while(str1[i]!='\0')
i++;
if(i>2)
write(2,"\nmore number of arguments has been given to cd\n",47);
else if(i==2)
{
if(strcmp(str1[1],"~")==0)
chdir(str3);
else
chdir(str1[1]);
}
else if(i==1)
chdir(str3);
}
//Finding out the original directory where the shell was run
void presentdir(char a[1000],char b[1000])
{
int i=0,pos,l=0,len;
len=strlen(a);
for(i=0;i<len;i++)
{
if(a[i]=='/')
pos=i;
}
pos++;
for(i=pos;i<len;i++)
b[l++]=a[i];
b[l]='\0';
}
//inputString has been split by spaces and stored in two dimensional array *cmdArgv[]
void parse(char *inputString, char *cmdArgv[])
{
int terminate=0,flag=0,cnt=0,cnt1=0;
char *srcPtr=inputString;
while(*srcPtr==' ' || *srcPtr=='\t' || *srcPtr=='\0' || *srcPtr=='\n')
srcPtr++;
while(*srcPtr!='\0')
{
if(*srcPtr=='<')
{
*cmdArgv='\0';
cnt1=0;
in=1;
*srcPtr='\0';srcPtr+=1;
while(*srcPtr==' ' || *srcPtr=='\n')
srcPtr++;
while(*srcPtr!=' ' && *srcPtr!='>' && *srcPtr!='\t' && *srcPtr!='\n')
{
filein[cnt1]=*srcPtr;
srcPtr++;
cnt1++;
if(*srcPtr==' ' || *srcPtr=='\n')
{
flag=1;
break;
}
}
filein[cnt1]='\0';
while(*srcPtr==' ')
srcPtr++;
printf("%d\n",flag);
if(flag)
break;
}
if(*srcPtr=='>')
{
*cmdArgv='\0';
out=1;cnt1=0;
*srcPtr='\0';srcPtr+=1;
while(*srcPtr==' ' || *srcPtr=='\0')
srcPtr++;
while(*srcPtr!=' ' && *srcPtr!='<' && *srcPtr!='\0' && *srcPtr!='\n')
{
fileout[cnt1]=*srcPtr;
srcPtr++;
cnt1++;
if(*srcPtr=='\0' || *srcPtr=='\n')
{
flag=1;
break;
}
}
if(*srcPtr==' ')
srcPtr++;
fileout[cnt1]='\0';
if(flag)
break;
}
if(*srcPtr!=' ' && *srcPtr!='&')
{
*cmdArgv = srcPtr;
}
while(*srcPtr!=' ' && *srcPtr!='\t' && *srcPtr!='\0' && *srcPtr!='\n')
{
//Checking whether the process is background or not and setting the flag accordingly
if(*srcPtr=='&')
{
*cmdArgv='\0';
bck=1;
*srcPtr='\0';
break;
}
srcPtr++;
}
if(flag)
break;
while((*srcPtr == ' ' || *srcPtr == '\t' || *srcPtr == '\n'))
{
*srcPtr = '\0';
srcPtr++;
if(*srcPtr=='\0')
break;
}
cmdArgv++;
}
*cmdArgv='\0';
}
//This function does the piping of different commands along with redirection operator if used
void pipeit(char *input)
{
int init,outit,i=0,l=0,cnt=0,j=0;
char *inp=input,*nextwordptr,doit[1000][1000];
/*for(l=0; *inp!='\0'; l++)
{
printf("%c",*inp);
inp+=1;
}
printf("%d\n",l);*/
/* Splitting the original command on basis of pipes */
//Parsing is done using the command strtok
nextwordptr=strtok(inp, "|");
while(nextwordptr!=NULL)
{
nextwordptr[strlen(nextwordptr)]='\0';
strcpy(doit[cnt], nextwordptr);
cnt++;
nextwordptr=strtok(NULL, "|");
}
//2 file descriptors are used to store the initial contents of STDIN and STDOUT
init=dup(STDIN_FILENO);
outit=dup(STDOUT_FILENO);
for(i=0;i<cnt;i++)
{
in=out=0;
char *doit1[1000];
parse(doit[i],doit1);
j=0;
/*while(doit1[j]!='\0')
{
printf("%s",doit1[j]);
j++;
}*/
//printf("%s\n",filein);
//calling the pipe function only if the last command is not present
//because the last command needs only input from the previous command but it's output will be displayed on stdout only
if(i<cnt-1)
pipe(newPipe);
//now we declare fork() to create the child process
pid=fork();
//if it's not the first command then only we provide input to it
//because first command doesn't need any input
if(i>0 && i<=cnt-1)
{
dup2(oldPipe[0],0);
close(oldPipe[0]);
close(oldPipe[1]);
}
//if it's a child process
if(pid==0)
{
//if it's not the last command then only we store it's output in newPipe
if(i>=0 && i<cnt-1)
{
dup2(newPipe[1],1);
close(newPipe[0]);
close(newPipe[1]);
}
//if < redirection is present then we push it onto a file
if(in==1)
{
FILE *fp=fopen(filein,"r");
dup2(fileno(fp),0);
in=0;
}
//if > redirection is present then we push it onto a file
if(out==1)
{
FILE *fp=fopen(fileout,"w+");
dup2(fileno(fp),1);
out=0;
}
//Now we finally execute the process
int x=execvp(*doit1,doit1);
if(x<0)
{
printf("No such command exists\n");
return;
}
}
//if it's the parent process
else if(pid>0)
{
int status;
waitpid(pid, &status, 0);
if(i<cnt-1)
{
oldPipe[0]=newPipe[0];
oldPipe[1]=newPipe[1];
}
}
else
{
printf("Unexpected ERROR\n");
}
}
close(oldPipe[0]);
close(newPipe[0]);
close(oldPipe[1]);
close(newPipe[1]);
// restore stdin, stdout
dup2(init, 0);
dup2(outit, 1);
close(init);
close(outit);
}
//Here ouput is flashed
void child_handler(int signo)
{	
int status;
int t=waitpid(-1,&status,WNOHANG),j;
if(t>0)
{
int flag=0;
//finding the process that has exited by searching for it in stack
for(j=0;j<stck;j++)
{
if(ar[j].p_id==t)
{
ar[j].status_value=0;
printf("\n%s with pid %d exited normally\n",*ar[j].name,ar[j].p_id);
doit();
fflush(stdout);
break;
}
}
}
signal(SIGCHLD,SIG_IGN);
}
//handles (ctrl+c) (ctrl+z) signals
void hand(int signo)
{
if(signo==20)
{
kill(pid,20);
}
else if(signo==2||signo==3)
{
fflush(stdout);
printf("\n");
doit();
fflush(stdout);
signal(SIGQUIT,hand);
signal(SIGTSTP,hand);
}
return ;
}
//The userdefined pinfo function is implemented here.
void pinfo(char *str1[1000])
{
int i=0,p;
char atr[1000],fin[1000],fin1[1000],fin2[1000];
while(str1[i]!='\0')
i++;
//error handling
if(i>2)
write(2,"\nmore number of arguments has been given to pinfo\n",47);
else if(i==2)
{
strcpy(atr,str1[1]);
}
else if(i==1)
{
p=getpid();
sprintf(atr,"%d",p);
}
if(i==2 || i==1)
{
int cnt=0,c;
size_t len=0;
ssize_t read;
char ch,*line;
//opening the /proc/<process_id>/status file which contains all the information about running process
strcpy(fin,"/proc/");
strcat(fin,atr);
strcpy(fin1,fin);
strcat(fin,"/status");
FILE *fp=fopen(fin, "r");
if(fp)
{
while ((read = getline(&line, &len, fp)) != -1)
{
cnt++;
if(cnt<=2 || cnt==4 || cnt==12)
printf("%s",line);
}
}
strcat(fin1,"exe");
if(readlink(fin1,fin2,sizeof(fin2)-1))
printf("Executable Path-- %s\n",fin2);
}
}
//Lists all the background jobs that are currently running on the system
void jobs()
{
int i,cnt=1,j;
for(i=0;i<stck;i++)
{
if(ar[i].status_value==1)
{
printf("[%d] ",cnt);
j=0;
while(strcmp(ar[i].name[j],"NULL")!=0)
{
printf("%s ",ar[i].name[j]);
j++;
}
printf("[%d]\n",ar[i].p_id);
cnt++;
}
}
}
//takes a job id of running job and sends a signal to it
void kjobs(int job, int sig)
{
int i=0,cnt=0;
//first the job is searched by using the job number from stack
//finally kill function is used
for(i=0;i<stck;i++)
{
if(ar[i].status_value==1)
cnt++;
if(cnt==job)
{
kill(ar[i].p_id, sig);
break;
}
}
}
//kills all the background processes
void overkill()
{
int i;
//all the running background process is searched from the stack and are killed using kill function
for(i=0;i<stck;i++)
{
if(ar[i].status_value==1)
{
kill(ar[i].p_id,9);
ar[i].status_value=0;
}
}
}
//brings the background process to foreground process
void fg(int ard)
{
int i,cnt=0,flag=0;
//the background process is searched from the stack
for(i=0;i<stck;i++)
{
if(ar[i].status_value==1)
cnt++;
if(cnt==ard)
{
flag=1;
int status,t;
ar[i].status_value=0;
//the id of the process exited is found and is printed after waiting for it to be finished
t=waitpid(ar[i].p_id,&status,0);
if(t>0)
printf("%s with pid %d exited normally\n",ar[i].name[0],ar[i].p_id);
}
}
if(flag==0)
write(2,"The process with this count doesn't exist\n",42);
}
//This function just checks whether pipe exists
int checkforpipe(char *stringit)
{
char *inp=stringit;
int flag=0;
while(*inp!='\n' && *inp!='\0')
{
if(*inp=='|')
{
pipeit(stringit);
flag=1;
break;
}
inp++;
}
return flag;
}
int main()
{
int t,len,val,i,l,j,lo,li;
char ch;
tcgetattr(STDIN_FILENO, &old);
tcgetattr(STDIN_FILENO, &new);
new.c_cc[VEOF]=3;
tcsetattr(0,TCSANOW,&new);
//current working directory is found out
getcwd(str3,1000);
//present directory name is stored in presdir
presentdir(str3,presdir);
while(1)
{
filein[0]=fileout[0]='\0';
in=out=lo=li=0;
int status;
size_t lengt=1000;
char *str1[1000];
str=(char*)malloc(sizeof(char)*1000);
for(i=0;i<1000;i++)
str1[i]=(char*)malloc(sizeof(char)*1000);
//all signals are being handled and send to child_handler or hand functions
signal(SIGCHLD,child_handler);
doit();
signal(SIGINT, SIG_IGN);
signal(SIGINT,hand);
signal(SIGQUIT,hand);
signal(SIGTSTP,hand);
bck=0;
getline(&str,&lengt,stdin);
//printf("%s\n",str);
if(checkforpipe(str))
continue;
if(strcmp(str,"quit\n")==0)
break;
len=strlen(str);
parse(str,str1);
/*for(i=0;str1[i]!='\0';i++)
{
printf("%s\n",str1[i]);
}
printf("%s",str1[1]);
printf("%d\n",i);*/
/*printf("i=%d\n",i);
printf("cut\n");
for(li=0;filein[li]!='\0';li++);
for(lo=0;fileout[lo]!='\0';lo++);
printf("%s %d\n",filein,li);
printf("%s %d\n",fileout,lo);
printf("in=%d\nout=%d\n",in,out);*/
j=0;i=0;
/*while(*(str1+j)!='\0')
{
printf("%s\n",*(str1+j));
j++;
}
j=0;*/
//checking it with all the user defined functions along with cd
if(strcmp(str1[0],"cd")==0)
doitcd(str1);
else if(strcmp(str1[0],"pinfo")==0)
pinfo(str1);
else if(strcmp(str1[0],"jobs")==0)
jobs();
else if(strcmp(str1[0],"kjob")==0)
kjobs(atoi(str1[1]),atoi(str1[2]));
else if(strcmp(str1[0],"overkill")==0)
overkill();
else if(strcmp(str1[0],"fg")==0)
fg(atoi(str1[1]));
else
{
//creating a new process
pid=fork();
if(pid==0)
{
if(in==1)
{
FILE *fp=fopen(filein,"r");
dup2(fileno(fp),0);
}
if(out==1)
{
FILE *fp=fopen(fileout,"w+");
dup2(fileno(fp),1);
}
//for vim manupulation
setpgid(0,0);
//if it's a child process, execute it
int x=execvp(*str1,str1);
//error handling
if(x<0)
printf("No such command exist\n");
exit(0);
}
else if(pid>0)
{
//checking whether it's background process or not
if(bck==0)
{
int stat1;
//checking whether (ctrl+z) is pressed or not
waitpid(pid,&stat1,WUNTRACED);
if(WIFSTOPPED(stat1))
{
j=0;
while(*(str1+j)!='\0')
{
strcpy(ar[stck].name[j],*(str1+j));
j++;
}
strcpy(ar[stck].name[j],"NULL");
ar[stck].p_id=pid;
ar[stck].status_value=1;
stck++;
}
}
//if it's a background process then it's accordingly put in stack
else
{
j=0;
while(*(str1+j)!='\0')
{
strcpy(ar[stck].name[j],*(str1+j));
j++;
}
strcpy(ar[stck].name[j],"NULL");
ar[stck].p_id=pid;
ar[stck].status_value=1;
stck++;
}
}
else
{
write(2,"Unexpected ERROR",16);
}
}
}
tcsetattr(0,TCSANOW,&old);
return 0;
}
