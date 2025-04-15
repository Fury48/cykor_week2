#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define COMMAND_SIZE 10

//bash의 유저네임과 호스트네임 선언
char username[50];
char hostname[50];
char path[50];

void bash(char* user, char* host);

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

	//프롬프트 입력받기
	while (1) {
		printf("%s@%s:", user, host);
		printf("%s", path);
		printf("$ ");
		fgets(command, COMMAND_SIZE, stdin);
		// 개행 문자 제거
		command[strcspn(command, "\n")] = '\0';
		if (strcmp(command, "exit") == 0) {
			printf("BASH를 종료합니다.");
			return;
		}
		else if (strcmp(command, "pwd") == 0) {
			printf("%s\n", path);
		}
		else if (strcmp(command, "ls") == 0) {
			printf("하위 디렉토리 출력(미구현)");
		}
		//chdir 통한 cd 구현
		else if (strncmp(command, "cd ", 3) == 0) {
			char* change_path = command + 3;
			if (chdir(change_path) != 0) {
				perror("디렉토리 변경 실패");
			}
			else {
				getcwd(path, sizeof(path));
			}

		}
		else printf("유효한 명령어가 없습니다\n");
		
	}
}
