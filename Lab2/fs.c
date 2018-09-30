/*
 * Unix Lab2
 * Author: Suwhan Kim
 * Student ID : 201510743
 */

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optind, opterr, optopt;

	// Check FileName is empty
	if (argv[optind] == NULL)
	{
		printf("fs: FileName is empty\n");
		return 0;
	}

	// Check accessible
	if (access(argv[optind], F_OK) != 0)
	{
		printf("fs: Cannot access file\n");
		return 0;
	}

	// Get file system information
	struct stat buffer;
	stat(argv[optind], &buffer);

	// Get file type
	char *fileType;
	switch (buffer.st_mode & S_IFMT)
	{
	case S_IFIFO:
		fileType = "FIFO";
		break;
	case S_IFCHR:
		fileType = "Character";
		break;
	case S_IFDIR:
		fileType = "Directory";
		break;
	case S_IFBLK:
		fileType = "Block";
		break;
	case S_IFREG:
		fileType = "Regular";
		break;
	case S_IFLNK:
		fileType = "Symbolic Link";
		break;
	case S_IFSOCK:
		fileType = "Socket";
		break;
	default:
		fileType = "Unknown";
		break;
	}

	// Get file mode
	unsigned int fileMode;
	fileMode = buffer.st_mode & 0xFFF;

	// Get file flags
	unsigned int fileFlags;
	fileFlags = buffer.st_mode & 07000;

	// Display file system information
	printf("  Inode: %d\t\t", (int)buffer.st_ino);
	printf("   Type: %s\n", fileType);
	printf("   Mode: %04o\t\t", fileMode);
	printf("  Flags: %04o\n", fileFlags);
	printf("   User: %d\t\t", (int)buffer.st_uid);
	printf("  Group: %d\t", (int)buffer.st_gid);
	printf("   Size: %d\n", (int)buffer.st_size);
	printf("  Links: %o\t\t", (unsigned int)buffer.st_nlink);
	printf("Blockcount: %d\n", (int)buffer.st_blocks);
	printf("  ctime: 0x%lx - %s", buffer.st_ctime, ctime(&buffer.st_ctime));
	printf("  atime: 0x%lx - %s", buffer.st_atime, ctime(&buffer.st_atime));
	printf("  mtime: 0x%lx - %s", buffer.st_mtime, ctime(&buffer.st_mtime));

	return 0;
}
