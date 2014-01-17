#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>

#define MAXLINE 100
#define BUF_SIZE 1024

void *cmd_cp(void *);
void *cmd_cat(void *);
void *cmd_ls(void *);
void *cmd_save(void *);
void *cmd_cd(void *);
void *cmd_logout(void *);
void *cmd_login(void *);
void *cmd_rmdir(void *);
void *cmd_rm(void *);
void *cmd_mkdir(void *);

typedef struct st_account {
	char id[MAXLINE];
	char pw[MAXLINE];
}acc;

void *
cmd_cp(void *name_args)
{
        char *name = (char *)name_args;
        char *source;
        char *dest;

        int source_fd;
        int dest_fd;
        int readn;
        int totaln=0;
        char buf[MAXLINE];

        source = strtok(name, " ");
        dest = strtok(NULL, " ");

        if( !(source_fd = open(source, O_RDONLY)))
        {
                return "Error";
        }

        if( !(dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644)))
        {
                return "Error";
        }

        if(errno == EEXIST)
        {
                close(dest_fd);
                return "Error";
        }

        memset(buf, 0x00, MAXLINE);
        while((readn = read(source_fd, buf, MAXLINE)) > 0)
        {
                printf("readn : %d\n", readn);
                totaln+=write(dest_fd, buf, readn);
                memset(buf, 0x00, MAXLINE);                      
        }


        return "copy completed";
}

void *
cmd_cat(void* name_arg)
{
        char *file_name = (char *)name_arg;
        char buf[BUF_SIZE] = {0,};
        int str_len;
        int fd;

        fd = open(file_name,O_RDONLY);

        while((str_len = read(fd, buf,BUF_SIZE))!=0);

        close(fd);

        return buf;
}

void *
cmd_ls(void *path_arg)
{
        DIR *d;
        char buf[BUF_SIZE] = {0,};
        char temp[MAXLINE] = {0,};      

        struct dirent *dent;
        int i, perm;

        d = opendir(".");

        if(!d) 
        {
                return "Can't";
        }       

        while(dent = readdir(d))
        {       
                sprintf(temp,"%s", dent->d_name);
                strcat(temp,"\n");
                strcat(buf, temp);
                memset((void *)&temp, 0x00, MAXLINE);
        }

        return buf;
}

void *
cmd_save(void* arg)
{
        char *msg = (char *)arg;
        char *temp;
        char *file_name;
        char *buf;
        int str_len, total_len;
        FILE* fp = NULL;
        char *dummy;
        
        temp = msg;
        puts(temp);
        int nMsg = strlen(temp) + 1;

        file_name = strtok(temp, " ");

        int nFile_name = strlen(file_name) + 1;
        int nBuf = nMsg - nFile_name;

        buf = (char *)malloc(nBuf + 1);

        int i = 0;

        for(i = 0; i < nBuf; ++i)
        {
                buf[i] = msg[i + nFile_name];
        }
        
        fp = fopen(file_name,"w");
        
        if(fp==NULL)
                return "save failed";

        fprintf(fp,"%s",buf);
        
        fclose(fp);

        strcat(file_name, " is created");
        
        return file_name;
}

void *
cmd_cd(void *args)
{
        char *arg = (char *)args;
        char *dirinfo;
        char *dirbuf = NULL;

        int argc = 1;

        dirinfo = arg;

        dirbuf = strtok(dirinfo, " ");
        puts(dirbuf);

        //pid = fork();

        if(dirbuf == NULL) 
        {
             chdir(getenv("HOME"));
        }
        else if(dirbuf != NULL)
        {
             chdir(dirbuf);
         }
        
        else
        {       
                return "Usage cd [dir]";
        }
        
        return "OK";
}

void *
cmd_rm(void *arg)
{
        char *filename = (char *)arg;
        int result = unlink(filename);

        if(result==0)
                ;
        else if(result ==-1)
                return "rm failed";

        strcat(filename, " is deleted");

        return filename;
}



void *
cmd_rmdir(void *arg)
{
		char *dirname = (char *)arg;

		puts(dirname);

        int result = rmdir(dirname);

        if(result==0)
                printf("rmdir success");
        else if(result ==-1)
                printf("rmdir fail");

        return "deleted";
	
}



void *
cmd_mkdir(void* arg)
{
        char *dirname = (char *)arg;

        int result = mkdir(dirname, 0777);

        if(result==0)
                ;
        else if(result==-1)
                return "mkdir failed";

        strcat(dirname, " is created");

        return dirname;
}



void *
cmd_logout(void *arg)
{
	int sock = *(int *)arg;

	shutdown(sock, SHUT_WR);
}

void *
cmd_login(void *arg)
{
		FILE *fp;
		acc user_account;
        char *loginfo = (char *)arg;
        char welcome_msg[MAXLINE] = "Welcome  !!";
        char id[MAXLINE] = {0,};
        char pw[MAXLINE] = {0,};
		int line_num = 0;
		int check = 0;
		char line_buff[MAXLINE] = {0, };

        sscanf(loginfo, "%s %s", id, pw);
	
		puts(id);
		puts(pw);
	
		if((fp = fopen("acc.txt","r")) == NULL)
		{
			return "can't access to account list";
		}
	
		while(0 < fscanf(fp, "%s %s", user_account.id, user_account.pw))
		{

			if((strcmp(user_account.id, id) == 0) && (strcmp(user_account.pw, pw) == 0))
			{
				strcat(welcome_msg, id);
				fclose(fp);	
				return welcome_msg;
			}
		}

		fclose(fp);
        return "Unauthorized";
}
