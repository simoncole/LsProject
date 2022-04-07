/*
Simon Cole
4/5/22
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>

int ls_file(char * fname);
int ls_dir(char * dname, int minusl);
int countEntries(DIR *dp);
void bubbleSort(char * strs[], int count);

struct stat mystat, *sp;
char * t1 ="xwrxwrxwr-------";
char * t2 = "----------------";

int main(int argc, char * argv[])
{
	struct stat mystat, *sp = &mystat;
	int r;
	int minusl = 0;
	char *filename, path[1024], cwd[256];
	filename = "./";
	if (argc == 2)
		filename = argv[1];
	else if(argc == 3){
		if(strcmp(argv[1], "-l") == 0){
			minusl = 1;
			printf("minusl out\n");
		}
		filename = argv[2];
	}
	
	if ((r = lstat(filename, sp)) < 0)
	{
		printf("No such file %s\n", filename);
		exit(1);
	}
	strcpy(path, filename);
	if (path[0] != '/')  //filename is relative
	{
		getcwd(cwd, 256);
		strcpy(path, cwd);
		strcat(path, "/");
		strcat(path, filename);
	}
	
	if(S_ISDIR(sp->st_mode))
		ls_dir(path, minusl);
	else
		ls_file(path);
	
	return 0;
}

int ls_file(char * fname)
{
	struct stat fstat, *sp;
	int r, i;
	char ftime[64];
	sp = &fstat;
	if ((r = lstat(fname, &fstat)) < 0)
	{
		printf("Can't stat %s\n", fname);
		return(1);
	}

	if((sp->st_mode & 0xF000) == 0x8000) // if S_ISREG()
		printf("%c", '-');
	if((sp->st_mode & 0xF000) == 0x4000) // if S_ISDIR()
		printf("%c", 'd');
	if((sp->st_mode & 0xF000) == 0xA000) // if S_ISLNK()
		printf("%c", 'l');

	for(i = 8; i >= 0; i--)
	{
		if (sp->st_mode & (1 << i))  //print r | w | x
			printf("%c", t1[i]);
		else
			printf("%c", t2[i]);  //print a -
	}
	
	printf("%4d ", sp->st_nlink);  //link count
	printf("%4d ", sp->st_gid);	//group id
	printf("%4d ", sp->st_uid);	// user id
	printf("%4lld ", sp->st_size); // file size
	
	strcpy(ftime, ctime(&sp->st_ctime));
	ftime[strlen(ftime)-1] = 0;
	printf("%s  ", ftime);
	
	// print name
	printf("%s", basename(fname)); 
	// print link name if file is a symbolic link file
	if ((sp->st_mode & 0xF000) == 0xA000)
	{
		//get link name using readlink()
		char linkname[256];
		ssize_t size = readlink(fname, linkname, 256);
		linkname[size] = '\0';
		printf(" -> %s", linkname);
	}
	printf("\n");
	return 0;
}

int ls_dir(char * dname, int minusl)
{
    DIR *dp;
    struct dirent *dirp;
	char filename[256];
	char ** dirEntries;
    dp = opendir(dname);
	int numInDir = countEntries(dp);
	printf("number of entries is: %d\n", numInDir);
	dirEntries = (char **)malloc(sizeof(char *) * numInDir);
	int i = 0;

	//make space and copy files in directory into dirEntries
    while ((dirp = readdir(dp)) != NULL) //countEntries looks like this
	{
		//create space here
		dirEntries[i] = (char *) malloc(sizeof(char) * dirp->d_namlen);
		strcpy(dirEntries[i], dirp->d_name);
		i++;
	}
	bubbleSort(dirEntries, numInDir);
	if(!minusl){
		for(i=2; i < numInDir; i++){ //start at 2 to skip the . and .. files
			if(strcmp(dirEntries[i], ".DS_Store") == 0)
				continue;
			printf("%s	", dirEntries[i]);
		}
	}
	else{
		for(i=0; i < numInDir; i++){
			strcpy(filename, dname);
			strcat(filename, "/");
			strcat(filename, dirEntries[i]);
			ls_file(filename);
		}
	}
		
    // closedir(dp);
	
	
	return 0;

}

int countEntries(DIR *dp){
	int count = 0;
	struct dirent *dirp;
	while ((dirp = readdir(dp)) != NULL){
		if(dirp->d_type == DT_REG){
			count++;
		}
	}
	closedir(dp);
	return count + 2; //add 2 for the . and .. files.
}

void bubbleSort(char * strs[], int count){
    int i, j;
    for(i=0; i < count; i++){
        for(j = 0; j < count-i-1; j++){
            if(strcmp(strs[j], strs[j+1]) > 0){
                char * temp = strs[j];
                strs[j] = strs[j+1];
                strs[j+1] = temp;
            }
        }
    }
}

	
