#include <stdio.h>
#include <string.h>
#include <direct.h>

#define COMMAND_SIZE 100

//bash�� �������Ӱ� ȣ��Ʈ���� ����
char username[50];
char hostname[50];
char path[50];

void bash(char* user, char* host);

int main(){
	//�������Ӱ� ȣ��Ʈ���� �Է¹ޱ�
	printf("Enter username:");
	scanf_s("%s", username,50u);
	printf("Enter hostname:");
	scanf_s("%s", hostname,50u);

	//�������� bash ����
	bash(username, hostname);

	return 0;
}

//������ bash���� �⺻���� �������̽� �� ��ɾ� �Է¹ޱ�
void bash(char* user, char* host) {
	char command[COMMAND_SIZE];
	int input_buffer;

	//���� ���丮�� /(c://)�� ����
	if (_chdir("/") != 0) {
		perror("���丮 ���� ����");
		return;
	}
	_getcwd(path, sizeof(path));

	// ���� ���� �� ���ۿ� ���� ���� ����
	while ((input_buffer = getchar()) != '\n' && input_buffer != EOF);

	//������Ʈ �Է¹ޱ�
	while (1) {
		printf("%s@%s:/", user, host);
		//....(��� ����)
		printf("$ ");
		fgets(command, COMMAND_SIZE, stdin);
		// ���� ���� ����
		command[strcspn(command, "\n")] = '\0';
		if (strcmp(command, "exit") == 0) {
			printf("BASH�� �����մϴ�.");
			return;
		}
		else if (strcmp(command, "pwd") == 0) {
			printf("%s\n", path);
		}
		
	}
}
