#include<stdio.h>
#include<sys/wait.h>
#include<string.h>
#include<time.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>

typedef struct {
	char* path;
	char* name;
} file_path; 

int same_files(file_path* file1, file_path* file2) {
	//TODO check permissions and content	
	return 0;
}

void check_duplicate_files(const char* filepath, file_path* *files, int files_size) {
	int i;	
	for(i = 0; i < files_size; i++) {
		//TODO do the actual check
	}

}

void print_file(const char* path, int output) {
	struct stat file_info;
	if(stat(path, &file_info) == 1) {
		fprintf(stderr, "Error getting data about a file. (%s)\n", strerror(errno));
		return;
	}

	//Prepare file info
	char serial_no[20], size[10], modification_date[20];
	sprintf(serial_no, "%u",(unsigned int) file_info.st_ino);
	write(output, serial_no, strlen(serial_no));
	sprintf(size, "%u", (unsigned int) file_info.st_size);
	write(output, " | ", strlen(" | "));
	strftime(modification_date, 20, "%g %b %e %H:%M", localtime(&file_info.st_mtime));
	write(output, modification_date, strlen(modification_date));
	write(output, " | ", strlen(" | "));
	write(output, size, strlen(size));
	write(output, " | ", strlen(" | "));
}

file_path* *read_from_file(const char* filepath, int* size) {
	int cur_size = 20;
	file_path* (*files) = (file_path**) malloc(cur_size*sizeof(file_path*));

	struct stat file_info;
	if(stat(filepath, &file_info) == -1) {
		fprintf(stderr, "The file could not be open. Exiting... (%s)\n", strerror(errno));
		exit(0);
	}

	FILE* file = fopen(filepath, "r");

	int i = -1;
	do {
		i++;
		if(i == cur_size -1) {
			cur_size += 20;
			files = (file_path**) realloc(files, cur_size*sizeof(file_path*));
		}
		files[i] = (file_path*) malloc(sizeof(file_path));	
		files[i]->path = (char*) malloc(200*sizeof(char));
		files[i]->name = (char*) malloc(50*sizeof(char));				
	} while(fscanf(file, "%s %s\n", files[i]->path, files[i]->name) != EOF);

	if(feof(file))
		printf("Acabou o ficheiro! Li %i cenas.\n", i);

	files = (file_path**) realloc(files, i*sizeof(file_path*));
	*size = i;

	return files;
}

int main(int argc, char* argv[]) {
	if(argc != 2) {
		fprintf(stderr, "Invalid number of arguments. \nProgram must be called as:\n./rmdup <directory>\nWhere <directory> is a valid directory that ends in \'/\'.\n");
		return 1;
	}

	const char* filepath = "./files.txt";

	int pid;
	if((pid = fork()) == 0) {
		execlp("./lsdir", "lsdir", argv[1], filepath, NULL);
	} else {
		waitpid(pid, NULL, 0);
	}

	int files_size = 0;
	//Not working properly with big files
   file_path* (*files) =	read_from_file(filepath, &files_size);
	printf("The array has size of %u.\n", files_size);

	//qsort(files, files_size, sizeof(file_path), comp_func);
	
	check_duplicate_files(filepath, files, files_size);

	return 0;
}
