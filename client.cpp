#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // 클라이언트 소켓 생성
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Failed to create client socket." << std::endl;
        return 1;
    }

    // 서버 주소 설정
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);

    // 서버에 연결
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to connect to server." << std::endl;
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    while (true) {
        // 서버로부터 보드 상태 수신 및 출력
        char boardMsg[225];
        recv(clientSocket, boardMsg, 225, 0);
        std::cout << "Current board state:" << std::endl;
        for (int i = 0; i < 15; ++i) {
            for (int j = 0; j < 15; ++j) {
                std::cout << boardMsg[i * 15 + j];
            }
            std::cout << std::endl;
        }

        // 돌 위치 입력 받기
        int row, col;
        std::cout << "Enter your move (row col): ";
        std::cin >> row >> col;

        // 서버로 돌 위치 전송
        send(clientSocket, &row, sizeof(int), 0);
        send(clientSocket, &col, sizeof(int), 0);
    }

    // 연결 종료
    close(clientSocket);

    return 0;
}

