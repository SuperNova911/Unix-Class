login as: 201510743
201510743@right.jbnu.ac.kr's password:
Last login: Mon Sep 10 10:19:46 2018 from 210.117.184.100
[201510743@right ~]$ ls
kappa  unix
[201510743@right ~]$ unix
-bash: unix: command not found
[201510743@right ~]$ cd unix
[201510743@right unix]$ ls
concat  concat.c  hello  hello.c
[201510743@right unix]$ concat
filename = (null)
[201510743@right unix]$ concat kappa
filename = kappa
[201510743@right unix]$ concat -n
-n option
filename = (null)
[201510743@right unix]$ concat -n kappa
-n option
filename = kappa
[201510743@right unix]$ more concat.c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int n;

    extern char *optarg;
    extern int optind, opterr, optopt;

    while ((n = getopt(argc, argv, "nl:h")) != -1)
    {
        switch (n)
        {
            case 'n':
                printf("-n option\n");
                break;
            case 'l':
                printf("-l option with %s\n", optarg);
                break;
            case 'h':
                printf("Usage: concat -h -l N -o outfile file\n");
                printf("    -h : Display help\n");
                printf("    -l : Display each N Lines\n");
                exit(0);
                break;
        }
    }

    printf("filename = %s\n", argv[optind]);

    return 0;
}
[201510743@right unix]$ clear
[201510743@right unix]$ chmod --help
사용법: chmod [옵션]... MODE[,MODE]... FILE...
  또는: chmod [옵션]... 8진수-MODE FILE...
  또는: chmod [옵션]... --reference=RFILE FILE...
Change the mode of each FILE to MODE.

  -c, --changes           like verbose but report only when a change is made
      --no-preserve-root  do not treat `/' specially (the default)
      --preserve-root     fail to operate recursively on `/'
  -f, --silent, --quiet   suppress most error messages
  -v, --verbose           output a diagnostic for every file processed
      --reference=RFILE   use RFILE's mode instead of MODE values
  -R, --recursive         change files and directories recursively
      --help     이 도움말을 표시하고 끝냅니다
      --version  버전 정보를 출력하고 끝냅니다

Each MODE is of the form `[ugoa]*([-+=]([rwxXst]*|[ugo]))+'.

Report chmod bugs to bug-coreutils@gnu.org
GNU coreutils home page: <http://www.gnu.org/software/coreutils/>
General help using GNU software: <http://www.gnu.org/gethelp/>
Report chmod translation bugs to <http://translationproject.org/team/>
For complete documentation, run: info coreutils 'chmod invocation'
[201510743@right unix]$ ls
concat  concat.c  hello  hello.c  kappa
[201510743@right unix]$ more concat.c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int n;

    extern char *optarg;
    extern int optind, opterr, optopt;

    while ((n = getopt(argc, argv, "nl:h")) != -1)
    {
        switch (n)
        {
            case 'n':
                printf("-n option\n");
                break;
            case 'l':
                printf("-l option with %s\n", optarg);
                break;
            case 'h':
                printf("Usage: concat -h -l N -o outfile file\n");
                printf("    -h : Display help\n");
                printf("    -l : Display each N Lines\n");
                exit(0);
                break;
        }
    }

vi -c 1 concat.c------------------------
    printf("filename = %s\n", argv[optind]);

    return 0;
}
[201510743@right unix]$
[201510743@right unix]$
[201510743@right unix]$ clear
[201510743@right unix]$ cat concat.c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int n;

    extern char *optarg;
    extern int optind, opterr, optopt;

    while ((n = getopt(argc, argv, "nl:h")) != -1)
    {
        switch (n)
        {
            case 'n':
                printf("-n option\n");
                break;
            case 'l':
                printf("-l option with %s\n", optarg);
                break;
            case 'h':
                printf("Usage: concat -h -l N -o outfile file\n");
                printf("    -h : Display help\n");
                printf("    -l : Display each N Lines\n");
                exit(0);
                break;
        }
    }

    printf("filename = %s\n", argv[optind]);

    return 0;
}
[201510743@right unix]$
