#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

const char *delim = " ";
const char *semicolon =  ";\n";
char home[1000];
int length_home_string;

typedef struct
{
	int id;
	char name[1000];
	int is_fg;
}proc;

proc Curr_proc;
char present_dir[1000]="~";
proc processes[1000];

int background_processes = 0;

void pwd()
{
	char *now = (char *)malloc(1000 * sizeof(char));
	
	char *code = getcwd(now, 1000);

	if(code == NULL)
		return;

	int i = 0; 
	int flag = 0;
	for( ; i < length_home_string ; ++i)
	{
		if(now[i] != home[i])
		{
			flag = 1;
			break;
		}
	}
	
	// checks if the directory is not in the subtree of the home directory

	if(flag == 1)
	{
		strcpy(present_dir, now);
		return;
	}

	int start_index = length_home_string;
	int end_index = strlen(now) - 1;

	char result[] = "~";

	for(i = start_index; i <= end_index; ++i)
		result[i - start_index + 1] = now[i];

	strcpy(present_dir, result);
}

void echo(char *recv)
{
	char *str = recv + 5;
	if((str[0] == '\"' && str[strlen(str)-2] == '\"') || (str[0] == '\'' && str[strlen(str)-2] == '\''))
	{
		char *updated_str = str + 1;
		str[strlen(updated_str) - 1] = '\0';
		printf("%s\n", updated_str);
		return;
	}
	printf("%s", str);
	return;
}

int cd(char *path)
{
	char before[1000];
	strcpy(before, present_dir);
	int i = 0;

	if(path[0] == '/' || path[0] == '~')
	{
		memmove(path, path + 1, sizeof path - sizeof *path);
		char new_path[1000] = "___###";
		strcpy(new_path, home);
		char slash[] = "/";
		strcat(new_path, slash);
		strcat(new_path, path);
		path = new_path;
	}

	int state = chdir(path);
	if(state != 0)
	{
		pwd();
		return 1;
	}
	
	char *then;
	then = (char *)malloc(1000 * sizeof(char));
	char *code = getcwd(then, 1000);

	if(code == NULL)
	{
		cd(before);
		return 1;
	}

	i = 0; 

	int flag = 0;
	for( ; i < length_home_string ; ++i)
	{
		if(then[i] != home[i])
		{
			flag = 1;
			break;
		}
	}
	
	pwd();
	return 0;
}

void run_ls(int a, int l, char *name, int is_name)
{
	char before[1000];
	strcpy(before, present_dir);

	if(is_name)
	{
		if(name[0] == '/' || name[0] == '~')
		{
			memmove(name, name + 1, sizeof name - sizeof *name);
			char new_path[1000] = "____##";
			strcpy(new_path, home);
			strcat(new_path, name);
			name = new_path;
		}
	}
	
	int flag = 0;
	struct dirent *myfile;
    struct stat mystat;
    char buf[2048];
    char *now = (char *)malloc(1000 * sizeof(char));
    int is_dir = 0;

   	char dot[10] = "."; 
   	char dotdot[10] = "..";

   	if(is_name != 0)
   	{
   		sprintf(buf, "%s", name);
        int got = stat(buf, &mystat);
        if(got == 0)
        {
	        if(S_ISDIR(mystat.st_mode))
	        {
			    int check = cd(name);
			    cd(before);
				if(check == 0 || check == 2)
					is_dir = 1;
			}
		}
   	}

	getcwd(now, 1000);
	DIR *mydir;

	if(is_dir == 0)
   		mydir = opendir(now);
   	else
   		mydir = opendir(name);

	while((myfile = readdir(mydir)) != NULL)
    {
    	if((strcmp(myfile->d_name, dotdot) == 0 || strcmp(myfile->d_name, dot) == 0) && a == 0)
    		continue;
    	if(is_name == 1 && is_dir == 0 && strcmp(name, myfile->d_name) != 0)
    		continue;
    	flag = 1;
    	if(l == 0)
    	{
    		printf("%s\n", myfile->d_name);
    		continue;
    	}
        sprintf(buf, "%s/%s", now, myfile->d_name);
        stat(buf, &mystat);
        printf((S_ISDIR(mystat.st_mode)) ? "d" : "-");
		printf((mystat.st_mode & S_IRUSR) ? "r" : "-");
		printf((mystat.st_mode & S_IWUSR) ? "w" : "-");
		printf((mystat.st_mode & S_IXUSR) ? "x" : "-");
		printf((mystat.st_mode & S_IRGRP) ? "r" : "-");
		printf((mystat.st_mode & S_IWGRP) ? "w" : "-");
		printf((mystat.st_mode & S_IXGRP) ? "x" : "-");
		printf((mystat.st_mode & S_IROTH) ? "r" : "-");
		printf((mystat.st_mode & S_IWOTH) ? "w" : "-");
		printf((mystat.st_mode & S_IXOTH) ? "x" : "-");
		printf("\t%d",(int)(mystat.st_nlink));
		printf("\t%s", getpwuid(mystat.st_uid)->pw_name);
		printf("\t%s", getgrgid(mystat.st_gid)->gr_name);
		printf("\t%zu",mystat.st_size);
	        
	    time_t t = time(NULL);
		struct tm *tmp = localtime(&t);
		char outstr[200];

		if(tmp->tm_year == localtime(&mystat.st_ctime)->tm_year)
			strftime(outstr, sizeof(outstr), "%b %e %R", localtime(&mystat.st_ctime));
		else
			strftime(outstr, sizeof(outstr), "%b %e %Y", localtime(&mystat.st_ctime));
		printf("\t%s", outstr);
		printf("\t%s\n", myfile->d_name);
	}

	if(flag == 0 && is_dir == 0 && is_name == 1)
		printf("ls: cannot access \'%s\': No such file or directory\n", name);
}

void handler(int sig)
{
	if(sig == SIGTSTP && Curr_proc.id >= 0)
	{
		processes[background_processes].id = Curr_proc.id;
		processes[background_processes].is_fg = 0;
		strcpy(processes[background_processes].name, Curr_proc.name);

		++background_processes;

		kill(Curr_proc.id, SIGTSTP);
		Curr_proc.id = -1;
		Curr_proc.is_fg = -1;
		strcpy(Curr_proc.name, "#invalid");
		signal(SIGTSTP, SIG_IGN);
	}

	if(sig == SIGINT)
	{
		int i=0;
		for(;i<background_processes;++i)
		{
			if(kill(processes[i].id, 0) == 0 && processes[i].is_fg == 1)
			{
				kill(processes[i].id, SIGINT);
				break;
			}
		}
		if(Curr_proc.id >= 0)
		{
			int dont_kill = 0;
			for(;i<background_processes;++i)
			{
				if(kill(processes[i].id, 0) == 0 && processes[i].is_fg == 0 && processes[i].id == Curr_proc.id)
					dont_kill = 1;
			}
			if(dont_kill == 0)
				kill(Curr_proc.id, 9);
			Curr_proc.id = -1;
			Curr_proc.is_fg = -1;
			strcpy(Curr_proc.name, "#invalid");
		}
		signal(SIGINT, SIG_IGN);
	}

	if(sig == SIGSTOP)
		signal(SIGSTOP, SIG_IGN);

	if(sig == SIGABRT)
		;

	fflush(stdout);
	return;
}

int pinfo(char* p_id, int is_arg)
{
	char process_name[1000] = "";
	strcpy(process_name, "/proc/");
	if(is_arg)
		strcat(process_name, p_id);
	else
	{
		sprintf(p_id, "%d", getpid());
		strcat(process_name, p_id);
	}

	char symbolic_path[1000] = "";
	strcat(symbolic_path, process_name);
	strcat(process_name, "/stat");
	strcat(symbolic_path, "/exe");

	char result[1000]="";

	int index = readlink(symbolic_path, result, sizeof(result));

	if(index == -1)
	{
		strcpy(result, "Link is Damaged");
		perror("Error in readlink");
		printf("%d\n", errno);
	}

	else
	{
		int i = 0; 
		int flag = 0;
		for( ; i < length_home_string ; ++i)
		{
			if(result[i] != home[i])
			{
				flag = 1;
				break;
			}
		}

		if(flag != 1)
		{
			int start_index = length_home_string;
			int end_index = strlen(result) - 1;

			char changed[] = "~";

			for(i = start_index; i <= end_index; ++i)
			{
				changed[i - start_index + 1] = result[i];
				changed[i - start_index + 2] = '\0';
			}

			strcpy(result, changed);
			
		}
	}

	int handle = open(process_name, O_RDONLY);

	if(handle == -1)
	{
		printf("Invalid Instruction\n");
		return -1;
	}

	char data[5000];
	read(handle, data, 500);

	char **tokenset = (char**)malloc(5000 * sizeof(char* ));
	char *token;
	char delims[] = " \t\r\n\a";

	int i = 0;
	token = strtok(data, delims);
	while(token != NULL)
	{
		tokenset[i] = token;
		++i;
		token = strtok(NULL, delims);
	}
	tokenset[i] = NULL;
	printf("pid -- %s\nProcess Status -- %s\nVirtual Memory-- %s\nExecutable Path -- %s\n", tokenset[0], tokenset[2], tokenset[23], result);
}

void wait_for_it(int wait, char *str)
{
	sleep(wait);
	char send_to[1000] = "echo ";
	strcat(send_to, str);
	send_to[strlen(send_to)] = ' ';
	send_to[strlen(send_to) + 1] = '\n';
	printf("Reminder: ");
	echo(send_to);
}

void print_time(int wait, int limit)
{
	int count = 0;
	while(count < limit)
	{
		time_t t = time(NULL);
		char outstr[200]; 
		strftime(outstr, sizeof(outstr), "%b %e %Y, %H:%M:%S ", localtime(&t));
		printf("%s\n", outstr);
		count += wait;
		sleep(wait);
	}
}

void run_em(char segments[100][100][100], int limit)
{
	int i = 0;
	int arr[2] = {0, 0};
	int inp_fd = 0;
	
	while(i < limit)
	{
		pipe(arr);
		int proc_id = fork();

		if(proc_id < 0)
		{
			printf("Internal Failure\n");
			return;
		}

		else if(proc_id == 0)
		{
			dup2(inp_fd, 0);
			close(arr[0]);
			if(i < limit - 1)
			{
				dup2(arr[1], 1);
			}

			int ip_fg = 0, op_fg = 0, ap_fg = 0;
			char ip_fname[128], op_fname[128], ap_fname[128];
			int file_des_ip, file_des_op, file_des_ap;
			int command_id;

			for(command_id = 0; strcmp(segments[i][command_id],"\0") != 0; ++command_id)
			{
				if(strcmp(segments[i][command_id], "<") == 0)
				{
					ip_fg = 1;
					strcpy(segments[i][command_id], "__##__\0");
					strcpy(ip_fname, segments[i][command_id + 1]);
					strcpy(segments[i][command_id + 1], "__##__\0");
				}

				if(strcmp(segments[i][command_id], ">>") == 0)
				{
					ap_fg = 1;
					strcpy(segments[i][command_id], "__##__\0");
					strcpy(ap_fname, segments[i][command_id + 1]);
					strcpy(segments[i][command_id + 1], "__##__\0");
				}

				else if(strcmp(segments[i][command_id], ">") == 0)
				{
					op_fg = 1;
					strcpy(segments[i][command_id], "__##__\0");
					strcpy(op_fname, segments[i][command_id + 1]);
					strcpy(segments[i][command_id + 1], "__##__\0");
				}
			}

			if(ip_fg == 1)
			{
				file_des_ip = open(ip_fname, O_RDONLY, 0);
				if(file_des_ip < 0)
				{
					printf("Error Opening Input File\n");
					continue;
				}
			}

			if(op_fg == 1)
			{
				file_des_op = open(op_fname, O_WRONLY | O_TRUNC | O_CREAT ,0644);
				if(file_des_op < 0)
				{
					printf("Error Opening Output File\n");
					continue;
				}
			}

			else if(ap_fg == 1)
			{
				file_des_ap = open(ap_fname, O_WRONLY | O_APPEND | O_CREAT, 0644);
				if(file_des_ap < 0)
				{
					printf("Error Opening Append File\n");
					continue;
				}
			}

			if(ip_fg == 1 || ap_fg == 1 || op_fg == 1)
			{
				char **args = (char **)malloc(sizeof(char *));
				int k = 0, idx = 0;
				for(; ; ++k)
				{

					if(strcmp(segments[i][k], "__##__\0") == 0)
						continue;
					if(strcmp(segments[i][k], "\0") == 0)
						break;
					args[idx] = (char *)malloc(sizeof(char));
					strcpy(args[idx], segments[i][k]);
					++idx;
				}
				args[idx] = NULL;

				int status;

				if(ip_fg == 1)
				{
					if(dup2(file_des_ip, 0) < 0)
					{
						printf("Error in dup\n");
						continue;
					}
				}
				
				if(ap_fg == 1)
				{
					if(dup2(file_des_ap, STDOUT_FILENO) < 0)
					{
						printf("Error in dup\n");
						continue;
					}
				}
				
				else if(op_fg == 1)
				{
					if(dup2(file_des_op, STDOUT_FILENO) < 0)
					{
						printf("Error in dup\n");
						continue;
					}
				}

				if(execvp(args[0], args) == -1)
					printf("Error\n");
				close(file_des_op);
				close(file_des_ap);
				close(file_des_ip);
				exit(1);
			}

			else
			{
				char **bargs = (char **)malloc(sizeof(char *));
				int k = 0;
				for(; ; ++k)
				{
					if(strcmp(segments[i][k],"\0") == 0)
						break;
					bargs[k] = (char *)malloc(sizeof(char));
					strcpy(bargs[k], segments[i][k]);
				}
				bargs[k]=NULL;
				execvp(bargs[0], bargs);
				exit(EXIT_FAILURE);
			}
		}

		else
		{
			wait(NULL);
			close(arr[1]);
			inp_fd = arr[0];
			++i;
		}
		int status;
		waitpid(proc_id, &status, WUNTRACED);
	}
}

void do_setenv(char var[1000], char var_val[1000])
{
	if(strcmp(var_val,"\0") == 0)
		setenv(var, " ", 1);
	else
		setenv(var, var_val, 1);
}

void jobs()
{
	int id, valid_procs = 0;
	for(id = 0; id < background_processes; ++id)
	{
		int living = kill(processes[id].id, 0);
		if(living == 0 && processes[id].is_fg == 0)
			++valid_procs;
			
	}
	printf("Number of Background Processes = %d\n", valid_procs);
	int i, count = 1;
	char proc_stat[100];
	for(i = 0; i < background_processes; ++i)
	{
		int living = kill(processes[i].id, 0);
		if(living == 0 && processes[i].is_fg == 0)
		{
			char process_name[1000] = "";
			strcpy(process_name, "/proc/");
			
			char p_id[100];
			sprintf(p_id, "%d", processes[i].id);
			strcat(process_name, p_id);

			strcat(process_name, "/stat");

			int handle = open(process_name, O_RDONLY);
			char data[5000];
			read(handle, data, 500);

			char **tokenset = (char**)malloc(5000 * sizeof(char* ));
			char *token;
			char delims[] = " \t\r\n\a";

			int idx = 0;
			token = strtok(data, delims);
			while(token != NULL)
			{
				tokenset[idx] = token;
				++idx;
				token = strtok(NULL, delims);
			}
			tokenset[idx] = NULL;
			char proc_stat[100];
			if(strcmp(tokenset[2], "T") == 0)
				strcpy(proc_stat, "STOPPED");
			else
				strcpy(proc_stat, "RUNNING");

			printf("[%d]\t%s\t%s\t[%d]\n", count, proc_stat, processes[i].name, processes[i].id);

			++count;
		}
	}
	return;
}

void kjob(int job_num, int sig_num)
{
	int i;
	int found = 0;
	for(i = 0; i < background_processes; ++i)
	{
		int check = kill(processes[i].id, 0);
		if(check == 0 && processes[i].is_fg == 0 && job_num == (i+1))
		{
			found = 1;
			kill(processes[i].id, sig_num);
			int stat;
			while(waitpid(-1, &stat, WNOHANG) > 0);
		}
	}
	if(found == 0)
		printf("Process Not Found\n");
	return;
}

void fg(int job_id)
{
	int i, found = 0, job_num = 0, stat;
	for(i = 0; i < background_processes; ++i)
	{
		if(kill(processes[i].id, 0) == 0 && job_id == (i+1) && processes[i].is_fg == 0)
		{
			found = 1;
			processes[i].is_fg = 1;
			Curr_proc = processes[i];
			kill(processes[i].id, SIGCONT);
			printf("Process with ID %d moved to foreground.\n", job_id);
			waitpid(processes[i].id, &stat, WUNTRACED);
		}
	}
	if(found == 0)
		printf("Process Not Found.\n");

	return;
}

void bg(int job_id)
{
	int i, found = 0, stat;
	for(i = 0; i < background_processes; ++i)
	{
		if(kill(processes[i].id, 0) == 0 && (i+1) == job_id && processes[i].is_fg == 0)
		{
			found = 1;
			kill(processes[i].id, SIGCONT);
			break;
		}
	}
	if(found == 0)
		printf("Invalid Job Number\n");
	return;
}

void overkill()
{
	int i;
	for(i = 0; i < background_processes; ++i)
		kill(processes[i].id, 9);
	return;
}

void quit()
{
	overkill();
	exit(0);
}

int main()
{
	getcwd(home, 1000);
	int out_defined, in_defined;
	in_defined = STDIN_FILENO;
	out_defined = STDOUT_FILENO;
	
	length_home_string = strlen(home);

	char *username;
	username = (char *)malloc(100 * sizeof(char));
	cuserid(username);

	char *machine_name;
	machine_name = (char *)malloc(100 * sizeof(char));
	gethostname(machine_name, 100);

	while(1)
	{
		signal(SIGINT, handler);
		signal(SIGTSTP, handler);
		signal(SIGSTOP, handler);
		signal(SIGABRT, handler);

		int pt;
		int count = 0;
		int temp_stat;
		proc arr[1000];
		while(waitpid(-1, &temp_stat, WNOHANG) > 0);

		for(pt = 0; pt < background_processes; ++pt)
		{
			int store = kill(processes[pt].id, 0);
			if(store == -1 && errno == ESRCH)
			{
				printf("%s with Process ID %d Exited Normally.\n", processes[pt].name, processes[pt].id);
				continue;
			}
			if(processes[pt].is_fg == 0)
			{
				arr[count] = processes[pt];
				++count;
			}
		}
		background_processes = count;
		for(pt = 0; pt <= background_processes; ++pt)
			processes[pt] = arr[pt];


		printf("<%s@%s:%s>", username, machine_name, present_dir);
		char *total = (char *)malloc(10000 * sizeof(char));
		char *temp_input = (char *)malloc(1000 * sizeof(char));

		size_t length = 0;

		Curr_proc.id = -1;
		Curr_proc.is_fg = -1;
		strcpy(Curr_proc.name, "#invalid");

		int got_back = getline(&total, &length, stdin);

		if(got_back <= 1)
			continue;

		temp_input[0] = 'a';

		int cnt = 0;
		int idx = 0;
		
		char **commands = (char **)malloc(1000 * sizeof(char *));

		temp_input = strtok(total, semicolon);

		while(temp_input != NULL)
		{
			commands[cnt++] = temp_input;
			temp_input = strtok(NULL, semicolon);
		}

		for(; idx < cnt ; ++idx)
		{
			char *input = (char *)malloc(1000 * sizeof(char));
			char temp[1000] = "\0";
			strcpy(input, commands[idx]);
			strcpy(temp, input);

			input[strlen(input)] = '\n';
			input[strlen(input) - 1] = ' ';
			input[strlen(input)] = '\0';

			char temp_input[1000];

			int index = 0, id;
			for(id = 0; id < strlen(input); ++id)
			{
				if(input[id] == '<')
				{
					temp_input[index++]=' ';
					temp_input[index++]='<';
					temp_input[index++]=' ';
					continue;
				}
				if(input[id] == '>')
				{
					if(id + 1 < strlen(input) && input[id + 1]=='>')
					{
						temp_input[index++]=' ';
						temp_input[index++]='>';
						temp_input[index++]='>';
						temp_input[index++]=' ';
						++id;
					}
					else
					{
						temp_input[index++]=' ';
						temp_input[index++]='>';
						temp_input[index++]=' ';
					}
					continue;
				}
				temp_input[index++]=input[id];
			}
			temp_input[index] = '\0';

			strcpy(input, temp_input);

			int command_id = 0, is_pipe = 0;

			for(; command_id < strlen(input); ++command_id)
			{

				if(input[command_id]=='|')
				{
					is_pipe = 1;
					break;
				}
			}
			
			if(is_pipe == 1)
			{
				char segments[100][100][100];
				char inp[10000];
				strcpy(inp, temp_input);
				char *t;
				char *tok = strtok_r(inp, "|", &t);
				int i = 0;

				while(tok != NULL)
				{
					int j = 0;
					char unit[10000];
					strcpy(unit, tok);
					char *t_a;
					char *unit_tokens = strtok_r(unit, delim, &t_a);
					while(unit_tokens != NULL)
					{
						strcpy(segments[i][j], unit_tokens);
						unit_tokens = strtok_r(NULL, delim, &t_a);
						++j;
					}
					strcpy(segments[i][j], "\0");
					tok = strtok_r(NULL, "|", &t);
					++i;
				}
				run_em(segments, i);
				continue;
			}



			int ip_fg = 0, op_fg = 0, ap_fg = 0;
			char ip_fname[128], op_fname[128], ap_fname[128];
			int file_des_ip, file_des_op, file_des_ap;

			char ip_segments[100][100];
			
			char *t;
			char *tok = strtok_r(temp_input, delim, &t);
			int i = 0;
			while(tok != NULL)
			{
				strcpy(ip_segments[i], tok);
				tok = strtok_r(NULL, delim, &t);
				++i;
			}
			strcpy(ip_segments[i], "__##__\0");

			for(command_id = 0; command_id < strlen(input); ++command_id)
			{
				if(strcmp(ip_segments[command_id], "<") == 0)
				{
					ip_fg = 1;
					strcpy(ip_segments[command_id], "\0");
					strcpy(ip_fname, ip_segments[command_id + 1]);
					strcpy(ip_segments[command_id + 1], "\0");
				}

				if(strcmp(ip_segments[command_id], ">>") == 0)
				{
					ap_fg = 1;
					strcpy(ip_segments[command_id], "\0");
					strcpy(ap_fname, ip_segments[command_id + 1]);
					strcpy(ip_segments[command_id + 1], "\0");
				}

				else if(strcmp(ip_segments[command_id], ">") == 0)
				{
					op_fg = 1;
					strcpy(ip_segments[command_id], "\0");
					strcpy(op_fname, ip_segments[command_id + 1]);
					strcpy(ip_segments[command_id + 1], "\0");
				}
			}

			if(ip_fg == 1)
			{

				file_des_ip = open(ip_fname, O_RDONLY, 0);
				if(file_des_ip < 0)
				{
					printf("Error Opening Input File\n");
					continue;
				}
			}

			if(op_fg == 1)
			{
				file_des_op = open(op_fname, O_WRONLY | O_TRUNC | O_CREAT ,0644);
				if(file_des_op < 0)
				{
					printf("Error Opening Output File\n");
					continue;
				}
			}

			else if(ap_fg == 1)
			{
				file_des_ap = open(ap_fname, O_WRONLY | O_APPEND | O_CREAT, 0644);
				if(file_des_ap < 0)
				{
					printf("Error Opening Append File\n");
					continue;
				}
			}

			if(ip_fg == 1 || ap_fg == 1 || op_fg == 1)
			{
				char **args = (char **)malloc(sizeof(char *));
				int k = 0, idx = 0;
				for(; ; ++k)
				{
					if(strcmp(ip_segments[k], "\0") == 0)
						continue;
					if(strcmp(ip_segments[k], "__##__\0") == 0)
						break;
					args[idx] = ip_segments[k];
					++idx;
				}
				args[idx] = NULL;
				int proc_id = fork();
				int status;

				if(proc_id < 0)
				{
					printf("Internal Failure\n");
					continue;
				}

				if(proc_id == 0)
				{
					if(ip_fg == 1)
					{
						if(dup2(file_des_ip, 0) < 0)
						{
							printf("Error in dup\n");
							continue;
						}
					}

					if(ap_fg == 1)
					{
						if(dup2(file_des_ap, STDOUT_FILENO) < 0)
						{
							printf("Error in dup\n");
							continue;
						}
					}

					else if(op_fg == 1)
					{
						if(dup2(file_des_op, STDOUT_FILENO) < 0)
						{
							printf("Error in dup\n");
							continue;
						}
					}

					if(execvp(args[0], args) == -1)
						printf("Error\n");
					close(file_des_op);
					close(file_des_ap);
					close(file_des_ip);
					exit(1);
				}
				waitpid(proc_id, &status, WUNTRACED);
	      		continue;
			}

			if(idx > 0)
				printf("\n-------------------------------------\n");

			if(strlen(input) <= 1)
				continue;

			char *tokenized = strtok(input, delim);

			int background = 0;
			char ampersand[10] = "&";

			char c_pwd[1000] = "pwd";

			int is_pwd = strcmp(c_pwd, tokenized);

			if(is_pwd == 0 && strtok(NULL, delim) == NULL)
			{
				int fault = 0;
				tokenized = strtok(NULL, delim);
				while(tokenized != NULL)
				{
					//printf("%s\n", tokenized);
					tokenized = strtok(NULL, delim);
					fault = 1;
				}
				if(fault == 1)
				{
					printf("Invalid Command\n");
					continue;
				}
				pwd();
				printf("%s\n", present_dir);
				continue;
			}

			char c_echo[1000] = "echo";

			int is_echo = strcmp(c_echo, tokenized);

			if(is_echo == 0)
			{
				echo(temp);
				tokenized = strtok(NULL, delim);
				while(tokenized != NULL)
					tokenized = strtok(NULL, delim);
				continue;
			}

			char c_cd[1000] = "cd";

			int is_cd = strcmp(c_cd, tokenized);		

			if(is_cd == 0)
			{
				tokenized = strtok(NULL, delim);
				if(tokenized != NULL)
				{
					char *hold = tokenized;
					int fault = 0;

					tokenized = strtok(NULL, delim);
					while(tokenized != NULL)
					{
						tokenized = strtok(NULL, delim);
						fault = 1;
					}
					if(fault == 1)
					{
						printf("Invalid Instruction\n");
						continue;
					}

					int value = cd(hold);
					if(value != 0)
						printf("Illegal Instruction\n");
				}
				else
					chdir(home);
				pwd();
				continue;
			}

			char c_ls[1000] = "ls";
			char c_a[1000] = "-a";
			char c_l[1000] = "-l";
			char c_al[1000] = "-al";
			char c_la[1000] = "-la";
			char *name;

			int is_ls = strcmp(c_ls, tokenized);

			if(is_ls == 0)
			{
				int flag_a = 0, flag_l = 0, is_name = 0;
				int dont_run = 0;
				tokenized = strtok(NULL, delim);
				while(tokenized != NULL)
				{
					if(strcmp(tokenized, c_a) == 0)
					{
						if(is_name == 1 || background == 1)
						{
							printf("Illegal Instruction\n");
							dont_run = 1;
							break;
						}
						flag_a = 1;
					}
					else if(strcmp(tokenized, c_l) == 0)
					{
						if(is_name == 1 || background == 1)
						{
							printf("Illegal Instruction\n");
							dont_run = 1;
							break;
						}
						flag_l = 1;
					}
					else if(strcmp(tokenized, c_la) == 0)
					{
						if(is_name == 1 || background == 1)
						{
							printf("Illegal Instruction\n");
							dont_run = 1;
							break;
						}
						flag_a = 1;
						flag_l = 1;
					}
					else if(strcmp(tokenized, c_al) == 0)
					{
						if(is_name == 1 || background == 1)
						{
							printf("Illegal Instruction\n");
							dont_run = 1;
							break;
						}
						flag_a = 1;
						flag_l = 1;
					}
					else if(strcmp(tokenized, ampersand) == 0)
					{
						if(is_name == 1 || background == 1)
						{
							printf("Illegal Instruction\n");
							dont_run = 1;
							break;
						}
						background = 1;
					}
					else
					{
						if(is_name == 1 || background == 1)
						{
							printf("Illegal Instruction\n");
							dont_run = 1;
							break;
						}
						name = tokenized;
						is_name = 1;
					}
					tokenized = strtok(NULL, delim);
				}

				if(dont_run == 1)
					continue;
				
				if(background == 0)
					run_ls(flag_a, flag_l, name, is_name);
				else
				{
					int pid = fork();
					if(pid == 0)
					{
						run_ls(flag_a, flag_l, name, is_name);
						exit(0);
					}
					int status;
					if(pid != 0)
					{
						processes[background_processes].id = (int)pid;
						strcpy(processes[background_processes].name, "ls");
						++background_processes;
						waitpid(pid, &status, WNOHANG);
					}
				}
				continue;
			}

			int is_pinfo = strcmp(tokenized, "pinfo");
			if(is_pinfo == 0)
			{
				int invalid = 0;
				char *p_id = (char *)malloc(1000 * sizeof(char));
				int is_arg = 0;
				int is_back = 0;
				tokenized = strtok(NULL, delim);
				while(tokenized != NULL)
				{
					if(strcmp("&", tokenized) == 0)
					{
						is_back = 1;
						tokenized = strtok(NULL, delim);
						continue;
					}
					
					if(is_arg == 1)
					{
						printf("Invalid Instruction\n");
						invalid = 1;
						break;
					}

					else
					{
						p_id = tokenized;
						is_arg = 1;
						tokenized = strtok(NULL, delim);
						continue;
					}
				}
				if(invalid == 1)
					break;
				if(is_arg == 1)
				{
					if(is_back == 0)
						pinfo(p_id, 1);
					else
					{
						int pid = fork();
						if(pid == 0)
						{
							pinfo(p_id, 1);
							exit(0);
						}
						int status;
						if(pid != 0)
						{
							++background_processes;
							waitpid(pid, &status, WNOHANG);
						}
					}
				}
				else
				{
					if(is_back == 0)
					{
						char *p_id = (char *)malloc(1000 * sizeof(char));
						pinfo(p_id, 0);
					}
					else
					{
						int pid = fork();
						if(pid == 0)
						{
							char *blank = (char *)malloc(1000 * sizeof(char));
							pinfo(blank, 1);
							exit(0);
						}
						int status;
						if(pid != 0)
						{
							processes[background_processes].id = (int)pid;
							strcpy(processes[background_processes].name, "pinfo");
							++background_processes;
							waitpid(pid, &status, WNOHANG);
						}
					}
				}
				continue;
			}

			int is_remind = strcmp("remindme", tokenized);

			if(is_remind == 0)
			{
				tokenized = strtok(NULL, delim);
				char time_wait[1000];
				char str[1000] = "";
				if(tokenized == NULL)
				{
					printf("Invalid Command\n");
					continue;
				}
				strcpy(time_wait, tokenized);
				tokenized = strtok(NULL, delim);
				if(tokenized == NULL)
				{
					printf("Invalid Command\n");
					continue;
				}
				strcpy(str, tokenized);
				tokenized = strtok(NULL, delim);
				while(tokenized != NULL)
				{
					strcat(str, " ");
					strcat(str, tokenized);
					tokenized = strtok(NULL, delim);
				}

				int pid = fork();
				int status;
				if(pid == 0)
				{
					wait_for_it(atoi(time_wait), str);
	      			exit(1);
				}
				processes[background_processes].is_fg = 0;
				processes[background_processes].id = (int)pid;
				strcpy(processes[background_processes].name, "remindme");
				++background_processes;
				waitpid(pid, &status, WNOHANG);
				continue;
			}

			int is_clock = strcmp("clock", tokenized);
			if(is_clock == 0)
			{
				tokenized = strtok(NULL, delim);
				if(tokenized == NULL)
				{
					printf("Invalid Command\n");
					continue;
				}
				if(strcmp("-t", tokenized) != 0)
				{
					printf("Invalid Command\n");
					continue;
				}
				tokenized = strtok(NULL, delim);
				if(tokenized == NULL)
				{
					printf("Invalid Command\n");
					continue;
				}
				int interval = atoi(tokenized);
				tokenized = strtok(NULL, delim);
				if(tokenized == NULL || interval < 1)
				{
					printf("Invalid Command\n");
					continue;
				}

				if(tokenized == NULL || strcmp("-n", tokenized) != 0)
				{
					printf("Invalid Command\n");
					continue;
				}
				tokenized = strtok(NULL, delim);
				if(tokenized == NULL)
				{
					printf("Invalid Command\n");
					continue;
				}
				int limit = atoi(tokenized);
				tokenized = strtok(NULL, delim);
				if(limit < 1)
				{
					printf("Invalid Command\n");
					continue;
				}

				int pid = fork();
				int status;
				if(pid == 0)
				{
					print_time(interval, limit);
	      			exit(1);
				}
				processes[background_processes].id = (int)pid;
				strcpy(processes[background_processes].name, "clock");
				processes[background_processes].is_fg = 0;
				++background_processes;
				waitpid(pid, &status, WUNTRACED);
				continue;
			}

			int is_setenv = strcmp("setenv", tokenized);
			if(is_setenv == 0)
			{
				tokenized = strtok(NULL, delim);
				if(tokenized == NULL)
				{
					printf("Too Few Arguments\n");
					continue;
				}
				printf("%s\n", tokenized);
				char var[1000] = "\0";

				strcpy(var, tokenized);
				tokenized = strtok(NULL, delim);
				char var_val[1000] = "\0";
				
				if(tokenized != NULL)
				{
					strcpy(var_val, tokenized);
					tokenized = strtok(NULL, delim);
					if(tokenized != NULL)
					{
						printf("Too Many Arguments\n");
						while(tokenized != NULL)
							tokenized = strtok(NULL, delim);
						continue;
					}
				}
				do_setenv(var, var_val);
				continue;
			}

			int is_unsetenv = strcmp("unsetenv", tokenized);
			if(is_unsetenv == 0)
			{
				tokenized = strtok(NULL, delim);
				if(tokenized == NULL)
				{
					printf("Too Few Arguments\n");
					continue;
				}
				char *var = (char *)malloc(1000 * sizeof(char));
				var = tokenized;
				int nope = 0;
				tokenized = strtok(NULL, delim);
				while(tokenized != NULL)
				{
					nope = 1;
					tokenized = strtok(NULL, delim);
				}
				if(nope == 1)
				{
					printf("Too Many Arguments");
					continue;
				}
				unsetenv(var);
				continue;
			}

			int is_jobs = strcmp("jobs", tokenized);
			if(is_jobs == 0)
			{
				tokenized = strtok(NULL, delim);
				if(tokenized != NULL)
				{
					printf("Too Many Arguments\n");
					continue;
				}
				jobs();
				continue;
			}

			int is_kjob = strcmp("kjob", tokenized);
			if(is_kjob == 0)
			{
				tokenized = strtok(NULL, delim);
				if(tokenized == NULL)
				{
					printf("Too Few Arguments\n");
					continue;
				}
				int job_num = atoi(tokenized);
				tokenized = strtok(NULL, delim);
				if(tokenized == NULL)
				{
					printf("Too Few Arguments\n");
					continue;
				}
				int sig_num = atoi(tokenized);
				tokenized = strtok(NULL, delim);
				if(tokenized != NULL)
				{
					printf("Too Many Arguments\n");
					while(tokenized != NULL)
						tokenized = strtok(NULL, delim);
					continue;
				}
				kjob(job_num, sig_num);
				continue;
			}

			int is_fg = strcmp("fg", tokenized);
			if(is_fg == 0)
			{
				tokenized = strtok(NULL, delim);
				if(tokenized == NULL)
				{
					printf("Too Few Arguments\n");
					continue;
				}
				int job_num = atoi(tokenized);
				int nope = 0;
				tokenized = strtok(NULL, delim);
				while(tokenized != NULL)
				{
					nope = 1;
					tokenized = strtok(NULL, delim);
				}
				if(nope == 1)
				{
					printf("Too Many Arguments\n");
					continue;
				}
				fg(job_num);
				continue;
			}

			int is_bg = strcmp("bg", tokenized);
			if(is_bg == 0)
			{
				tokenized = strtok(NULL, delim);
				if(tokenized == NULL)
				{
					printf("Too Few Arguments\n");
					continue;
				}
				int job_num = atoi(tokenized);
				int nope = 0;
				tokenized = strtok(NULL, delim);
				while(tokenized != NULL)
				{
					nope = 1;
					tokenized = strtok(NULL, delim);
				}
				if(nope == 1)
				{
					printf("Too Many Arguments\n");
					continue;
				}
				bg(job_num);
				continue;	
			}

			int is_overkill = strcmp("overkill", tokenized);
			if(is_overkill == 0)
			{
				while(tokenized != NULL)
					tokenized = strtok(NULL, delim);
				overkill();
				continue;
			}

			int is_quit = strcmp("quit", tokenized);
			if(is_quit == 0)
			{
				while(tokenized != NULL)
					tokenized = strtok(NULL, delim);
				quit();
				continue;
			}

			char **args = (char **)malloc(sizeof(char *));
			args[0] = tokenized;
			tokenized = strtok(NULL, delim);
			index = 1;

			while(tokenized != NULL)
			{
				args[index] = tokenized;
				++index;
				tokenized = strtok(NULL, delim);
			}



			if(strcmp(args[index - 1], "&") == 0)
			{
				args[index - 1] = NULL;
				int pid = fork();
				int status;
				if(pid == 0)
				{
					setpgid(0,0);
					if(execvp(args[0], args) == -1)
	      				printf("Invalid Command\n");
	      			exit(1);
				}
				processes[background_processes].is_fg = 0;
				processes[background_processes].id = (int)pid;
				strcpy(processes[background_processes].name, args[0]);
				++background_processes;
				waitpid(pid, &status, WNOHANG);
				continue;
			}

			else
			{
				int pid = fork();
				int status;
				args[index] = NULL;
				if(pid == 0)
				{
					setpgid(0,0);
					if(execvp(args[0], args) == -1)
	      				printf("Invalid Command\n");
	      			exit(1);
				}
				Curr_proc.is_fg = 1;
				strcpy(Curr_proc.name, args[0]);
				Curr_proc.id = pid;
				waitpid(pid, &status, WUNTRACED);
	      		continue;
			}
			pwd();
		}
	}
}
