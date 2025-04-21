#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#define COMMAND_SIZE 256

//bash의 유저네임과 호스트네임 선언
char username[50];
char hostname[50];
char path[50];

//디렉토리 관련 구조체 변수 선언
DIR* dir;
struct dirent* ent;

//함수 원형 선언
void bash(char* user, char* host);
int command_process(char* str);
void remove_space(char* b); 

int main(){
	//유저네임과 호스트네임 입력받기
	printf("Enter username:");
	scanf("%s", username);
	printf("Enter hostname:");
	scanf("%s", hostname);

	//본격적인 bash 실행
	bash(username, hostname);
	
	return 0;
}

//리눅스 bash셸의 기본적인 인터페이스 및 명령어 입력받기
void bash(char* user, char* host) {
	char command[COMMAND_SIZE];
	int input_buffer;
	

	//현재 디렉토리를 /(c://)로 설정
	if (chdir("/") != 0) {
		perror("디렉토리 변경 실패");
		return;
	}
	getcwd(path, sizeof (path));

	// 최초 실행 시 버퍼에 남은 개행 제거
	while ((input_buffer = getchar()) != '\n' && input_buffer != EOF);

	//명령어 입력받기 프롬프트
	while (1) {
		//프롬프트 형식
		dir = opendir(".");
		printf("%s@%s:", user, host);
		printf("%s", path);
		printf("$ ");
		fgets(command, COMMAND_SIZE, stdin);
		// 개행 문자 제거
		command[strcspn(command, "\n")] = '\0';
		if(command_process(command) == 1) break;
	}
}
// 문장의 앞 뒤 공백 삭제
void remove_space(char* b) {
    int i=0;
    while(b[i] == ' ') i++;
    if (i > 0) memmove(b, b + i, strlen(b + i) + 1);

	int len = strlen(b);
    while (len > 0 && b[len - 1] == ' ') {
        b[len - 1] = '\0';
        len--;
	}
}
//명령어 수행 함수
int command_process(char* str) {
	remove_space(str);

	//다중명령어1: ; 구현
	if (strstr(str,";")!=NULL) {
		char* bookmark = strstr(str,";");
		char front_command[COMMAND_SIZE], back_command[COMMAND_SIZE];
		int bookmark_length = bookmark - str;

		strncpy(front_command,str,bookmark_length);
		front_command[bookmark_length] = '\0';
		remove_space(front_command);

		strcpy(back_command,bookmark+1);
		remove_space(back_command);

		command_process(front_command);
		command_process(back_command);

		return 0;
	}
	
	// exit 구현
	else if (strcmp(str, "exit") == 0) {
		printf("BASH를 종료합니다.\n");
		return 1;
	}
	//pwd 구현
	else if (strcmp(str, "pwd") == 0) {
		printf("%s\n", path);
	}
	//chdir 통한 cd 구현
	else if (strncmp(str, "cd ", 3) == 0) {
		char* change_path = str + 3;
		if (chdir(change_path) != 0) {
			perror("디렉토리 변경 실패");
		}
		else {
			getcwd(path, sizeof(path));
		}

	}
	// ls 구현
	else if (strcmp(str, "ls") == 0) {
		while ((ent = readdir(dir)) != NULL) {
			printf("%s\n", ent->d_name);
		}
	}
	return 0;
}

