#include <syslog.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/sysinfo.h>
//#include <math.h>

void sig_handler(int signo)
{
  if(signo == SIGTERM)
  {
    syslog(LOG_INFO, "SIGTERM has been caught! Exiting...");
    if(remove("run/daemon.pid") != 0)
    {
      syslog(LOG_ERR, "Failed to remove the pid file. Error number is %d", errno);
      exit(1);
    }
    exit(0);
  }
}

void handle_signals()
{
  if(signal(SIGTERM, sig_handler) == SIG_ERR)
  {
    syslog(LOG_ERR, "Error! Can't catch SIGTERM");
    exit(1);
  }
}
  
void daemonise()
{
  pid_t  sid;
  FILE *pid_fp;
pid_t pid;  
syslog(LOG_INFO, "Starting daemonisation.");

  //First fork
  pid = fork();
  if(pid < 0)
  {
    syslog(LOG_ERR, "Error occured in the first fork while daemonising. Error number is %d", errno);
    exit(1);
  }

  if(pid > 0)
  {
    syslog(LOG_INFO, "First fork successful (Parent)");
    exit(0);
  }
  syslog(LOG_INFO, "First fork successful (Child)");

  //Create a new session
  sid = setsid();
  if(sid < 0) 
  {
    syslog(LOG_ERR, "Error occured in making a new session while daemonising. Error number is %d", errno);
    exit(1);
  }
  syslog(LOG_INFO, "New session was created successfuly!");

  //Second fork
  pid = fork();
  if(pid < 0)
  {
    syslog(LOG_ERR, "Error occured in the second fork while daemonising. Error number is %d", errno);
    exit(1);
  }

  if(pid > 0)
  {
    syslog(LOG_INFO, "Second fork successful (Parent)");
    exit(0);
  }
  syslog(LOG_INFO, "Second fork successful (Child)");

  pid = getpid();

  //Change working directory to Home directory
  if(chdir(getenv("HOME")) == -1)
  {
    syslog(LOG_ERR, "Failed to change working directory while daemonising. Error number is %d", errno);
    exit(1);
  }

  //Grant all permisions for all files and directories created by the daemon
  umask(0);

  //Redirect std IO
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  if(open("/dev/null",O_RDONLY) == -1)
  {
    syslog(LOG_ERR, "Failed to reopen stdin while daemonising. Error number is %d", errno);
    exit(1);
  }
  if(open("/dev/null",O_WRONLY) == -1)
  {
    syslog(LOG_ERR, "Failed to reopen stdout while daemonising. Error number is %d", errno);
    exit(1);
  }
  if(open("/dev/null",O_RDWR) == -1)
  {
    syslog(LOG_ERR, "Failed to reopen stderr while daemonising. Error number is %d", errno);
    exit(1);
  }

  //Create a pid file
  mkdir("run/", 0777);
  pid_fp = fopen("run/daemon.pid", "w");
  if(pid_fp == NULL)
  {
    syslog(LOG_ERR, "Failed to create a pid file while daemonising. Error number is %d", errno);
    exit(1);
  }
  if(fprintf(pid_fp, "%d\n", pid) < 0)
  {
    syslog(LOG_ERR, "Failed to write pid to pid file while daemonising. Error number is %d, trying to remove file", errno);
    fclose(pid_fp);
    if(remove("run/daemon.pid") != 0)
    {
      syslog(LOG_ERR, "Failed to remove pid file. Error number is %d", errno);
    }
    exit(1);
  }
  fclose(pid_fp);
}
//randomDigitForFile = (sys.totalram)|(sys.freeram)&(sys.sharedram)|(sys.bufferram)&(sys.totalswap)&(sys.freeswap>>sys.uptime);


int randomDigit(){
	struct sysinfo sys;
	sysinfo(&sys);
	return (int) (rand()^(clock()*sys.bufferram));
		//(int) (rand()^(clock()*sys.bufferram)^sys.procs);
}

int checkSize(){
FILE *fileForDigit;
   chdir(getenv("HOME"));
   fileForDigit = fopen("random/buf","r");
   int sizeF = 0, curPos = 0;
   curPos = ftell(fileForDigit);
   fseek(fileForDigit,0,SEEK_END);
   sizeF = ftell(fileForDigit);
   fseek(fileForDigit,curPos,SEEK_SET);
   fclose(fileForDigit);
   return sizeF;
}
int main(int argc, char **argv[])
{
  FILE *file;
  pid_t pid;
  if (argc != 2){
     ///fprintf(stderr,"NOT ENOUGH ARGUMENTS");
     syslog(LOG_INFO,"NOT ENOUGH ARGUMENTS",errno); 
     exit(1);
  } 
  if (!(strcmp(argv[1],"start"))){
     chdir(getenv("HOME"));
     if ((file = fopen("run/daemon.pid","r")) == NULL){
     syslog(LOG_INFO,"fopen FILE IF",errno);
     daemonise();
     handle_signals();
    }else{
      fclose(file);
      exit(1);
   }
  }
chdir(getenv("HOME"));
mkdir("random",S_IRWXU | S_IRWXG | S_IRWXO);
FILE *fileForDigit;



int digit = 0;
  if (!(strcmp(argv[1],"stop"))){
     chdir(getenv("HOME"));
     if ((file = fopen("run/daemon.pid","r")) == NULL){
	return 0;
    }
   else{
      fscanf(file,"%d",&pid);
      syslog(LOG_INFO,"in IF CLOSE",errno);
      fclose(file);
      kill(pid,SIGTERM);
      exit(0);
   }
  }

  while (1){ 
   sleep(5);
   fileForDigit = fopen("random/buf","ab");
   while (checkSize() < 1024*1024*5){
        digit = randomDigit();
	fwrite(&digit,sizeof(int),1,fileForDigit); 
   }
   fclose(fileForDigit);
  }
  return 0;
}
