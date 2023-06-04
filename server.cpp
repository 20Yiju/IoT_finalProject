#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

const int BOARD_SIZE = 15;

enum class BoardState {
    EMPTY,
    BLACK,
    WHITE
};

class Board {
public:
    std::vector<std::vector<BoardState>> board;

    Board() : board(BOARD_SIZE, std::vector<BoardState>(BOARD_SIZE, BoardState::EMPTY)) {}

    void placeStone(int row, int col, BoardState state) {
        board[row][col] = state;
    }

    const std::vector<std::vector<BoardState>>& getBoard() const {
        return board;
    }
};

void sendBoardState(int clientSocket, const Board& board) {
    std::string boardMsg;
    const std::vector<std::vector<BoardState>>& gameBoard = board.getBoard();
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            switch (gameBoard[i][j]) {
                case BoardState::EMPTY:
                    boardMsg += ".";
                    break;
                case BoardState::BLACK:
                    boardMsg += "B";
                    break;
                case BoardState::WHITE:
                    boardMsg += "W";
                    break;
            }
        }
    }
    send(clientSocket, boardMsg.c_str(), boardMsg.size(), 0);
}

int main() {
    int serverSocket, clientSocket1, clientSocket2;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen;

    // 서버 소켓 생성
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create server socket." << std::endl;
        return 1;
    }

    // 서버 주소 설정
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8888);

    // 서버에 바인딩
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind server socket." << std::endl;
        return 1;
    }

    // 클라이언트 연결 대기
    if (listen(serverSocket, 2) < 0) {
        std::cerr << "Failed to listen for client connections." << std::endl;
        return 1;
    }

    std::cout << "Waiting for players to connect..." << std::endl;

    // 첫 번째 클라이언트 연결
    clientAddrLen = sizeof(clientAddr);
    clientSocket1 = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket1 < 0) {
        std::cerr << "Failed to accept client connection." << std::endl;
        return 1;
    }
    std::cout << "Player 1 connected." << std::endl;

    // 두 번째 클라이언트 연결
    clientSocket2 = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket2 < 0) {
        std::cerr << "Failed to accept client connection." << std::endl;
        return 1;
    }
    std::cout << "Player 2 connected." << std::endl;

    Board board;

    // 게임 시작
    while (true) {
        // 플레이어 1에게 보드 상태 전송
        sendBoardState(clientSocket1, board);

        // 플레이어 1의 돌 위치 수신 및 처리
        int player1Row, player1Col;
        recv(clientSocket1, &player1Row, sizeof(int), 0);
        recv(clientSocket1, &player1Col, sizeof(int), 0);
        board.placeStone(player1Row, player1Col, BoardState::BLACK);

        // 게임 보드 출력
        std::cout << "Player 1 (Black) moved: (" << player1Row << ", " << player1Col << ")" << std::endl;

        // 플레이어 2에게 보드 상태 전송
        sendBoardState(clientSocket2, board);

        // 플레이어 2의 돌 위치 수신 및 처리
        int player2Row, player2Col;
        recv(clientSocket2, &player2Row, sizeof(int), 0);
        recv(clientSocket2, &player2Col, sizeof(int), 0);
        board.placeStone(player2Row, player2Col, BoardState::WHITE);

        // 게임 보드 출력
        std::cout << "Player 2 (White) moved: (" << player2Row << ", " << player2Col << ")" << std::endl;
    }

    // 연결 종료
    close(clientSocket1);
    close(clientSocket2);
    close(serverSocket);

    return 0;
}

