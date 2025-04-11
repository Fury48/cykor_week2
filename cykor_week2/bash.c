#include <stdio.h>
#include <string.h>
#define COMMAND_SIZE 100

//bash의 유저네임과 호스트네임 선언
char username[50];
char hostname[50];
char prompt[50][50];

void bash(char* user, char* host);

int main(){
	//유저네임과 호스트네임 입력받기
	printf("Enter username:");
	scanf_s("%s", username,50u);
	printf("Enter hostname:");
	scanf_s("%s", hostname,50u);

	//프롬프트에 유저네임과 호스트네임 추가
	sprintf_s(prompt[0], sizeof(prompt[0]), "%s", username);
	sprintf_s(prompt[1], sizeof(prompt[1]), "%s", username);


	//본격적인 bash 실행
	bash(username, hostname);

	return 0;
}

//리눅스 bash셸의 기본적인 인터페이스 및 명령어 입력받기
void bash(char* user, char* host) {
	char command[COMMAND_SIZE];
	int input_buffer;

	// 최초 실행 시 버퍼에 남은 개행 제거
	while ((input_buffer = getchar()) != '\n' && input_buffer != EOF);

	//프롬프트 입력받기
	while (1) {
		printf("%s@%s:/", user, host);
		//....(경로 구현)
		printf("$ ");
		fgets(command, COMMAND_SIZE, stdin);
		// 개행 문자 제거
		command[strcspn(command, "\n")] = '\0';
		if (strcmp(command, "exit") == 0) {
			printf("BASH를 종료합니다.");
			return;
		}
		printf("%s\n", command);

	}
}
