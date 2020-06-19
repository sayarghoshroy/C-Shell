# C shell

## Basic unix shell functionality implemented in C

<p align="justify">User-defined interactive shell program that can create and manage new processes. The shell is able to create processes out of system programs like emacs, vi or any user-defined executable.</p>

#### To compile:

```bash
make shell
```

#### To run:
```bash
./shell
```

## Instructions

- \<username@system_name:current_directory\> is the format of the displayed prompt.

- <p align="justify">The directory from which the shell is invoked is the home directory of the shell & is indicated by '~'.</p>

- 'cd' to change directory.

- 'echo' to display a string of text.

- 'pwd' to display the present working directory.

- <p align="justify">'ls' to list directory contents with flags 'a' and 'l' for all content including hidden files & content displayed in long form respectively.</p>

- '&' to designate a command as a background process.

- 'pinfo' displays process related information of the shell program. Usage: 'pinfo \<process_id\>'

- <p align="justify">Appropriate messages are given to the user when a background process exits. For example, 'emacs with pid 456 exited normally'.</p>

- <p align="justify">'remindme' displays a custom message to the user after a specified number of seconds. For example, 'remindme 1000 “Water the Plants”'.</p>

- <p align="justify">'clock' command to display the current date and time after fixed intervals of seconds. For example, 'clock -t 5' yields:</p>

```
  10 Sep 2020, 05:10:20
  10 Sep 2020, 05:10:25
  10 Sep 2020, 05:10:30
```

- Input-Output Redirection using '\<', '\>', and '\>\>'. For example, 'sort \< lines.txt \>\> sortedlines.txt'

- Command redirection using pipes identified by '|'. For example, 'more file.txt | wc'

- <p align="justify">'setenv' to create if needed and assign values to shell environment variables. Initially, the shell inherits environment variables from its parent. Usage: 'setenv \<variable\> \<value\>'</p>

- 'unsetenv' to destroy a shell environment variable. Usage: 'unsetenv \<variable\>'

- <p align="justify">'jobs' to print a list of all currently running jobs along with their process id and state in order of their times of creation.</p>

- 'kjob' to send signals to particular jobs. Usage: 'kjob \<job_id\> \<signal\>'

- 'fg' to bring a running or a stopped background job with given job id to foreground. Usage: 'fg <job_id>'

- 'bg' to change status of a stopped background job to running. Usage: 'bg \<jobNumber\>'

- 'overkill' to kill all background processes.

- 'quit' to exit the shell.

- <p align="justify"><kbd>ctrl</kbd> + <kbd>Z</kbd> changes the status of currently running job to 'stopped' and makes it a background process.</p>

- <kbd>ctrl</kbd> + <kbd>C</kbd> sends a SIGINT signal to the current foreground job.

## Implementation

- No popen, pclose, or system() calls were used.

-  <p align="justify">The user can type in any command, including './a.out' which starts a new process. The shell is able to execute the command or show the appropriate error message if the command cannot be executed.</p>

- The user can type the command anywhere in the command line with any number of spaces or tabs.

---
