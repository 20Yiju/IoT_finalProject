#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

enum class BoardState {
    EMPTY,
    BLACK,
    WHITE
};

enum class GameResult {
    TIE,
    PLAYER1_WIN,
    PLAYER2_WIN
};

class Board {
public:
    Board() {
        board.resize(15, std::vector<BoardState>(15, BoardState::EMPTY));
    }

    void placeStone(int row, int col, BoardState stone) {
        board[row][col] = stone;
    }

    const std::vector<std::vector<BoardState>>& getBoard() const {
        return board;
    }

private:
    std::vector<std::vector<BoardState>> board;  // 오목 게임 보드
};

bool isGameOver(const std::vector<std::vector<BoardState>>& gameBoard, int row, int col, BoardState stone) {
    // 가로 방향 확인
    int count = 1;
    int i = row, j = col - 1;
    while (j >= 0 && gameBoard[i][j] == stone) {
        count++;
        j--;
    }
    j = col + 1;
    while (j < 15 && gameBoard[i][j] == stone) {
        count++;
        j++;
    }
    if (count >= 5) {
        return true;
    }

    // 세로 방향 확인
    count = 1;
    i = row - 1, j = col;
    while (i >= 0 && gameBoard[i][j] == stone) {
        count++;
        i--;
    }
    i = row + 1;
    while (i < 15 && gameBoard[i][j] == stone) {
        count++;
        i++;
    }
    if (count >= 5) {
        return true;
    }

    // 대각선 방향 확인 (왼쪽 상단에서 오른쪽 하단)
    count = 1;
    i = row - 1, j = col - 1;
    while (i >= 0 && j >= 0 && gameBoard[i][j] == stone) {
        count++;
        i--;
        j--;
    }
    i = row + 1, j = col + 1;
    while (i < 15 && j < 15 && gameBoard[i][j] == stone) {
        count++;
        i++;
        j++;
    }
    if (count >= 5) {
        return true;
    }

    // 대각선 방향 확인 (왼쪽 하단에서 오른쪽 상단)
    count = 1;
    i = row + 1, j = col - 1;
    while (i < 15 && j >= 0 && gameBoard[i][j] == stone) {
        count++;
        i++;
        j--;
    }
    i = row - 1, j = col + 1;
    while (i >= 0 && j < 15 && gameBoard[i][j] == stone) {
        count++;
        i--;
        j++;
    }
    if (count >= 2) {
        return true;
    }

    return false;
}

void sendBoardState(int clientSocket, const Board& board) {
    std::vector<std::vector<BoardState>> gameBoard = board.getBoard();
    std::string boardMsg;
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            if (gameBoard[i][j] == BoardState::EMPTY) {
                boardMsg += ".";
            } else if (gameBoard[i][j] == BoardState::BLACK) {
                boardMsg += "B";
            } else if (gameBoard[i][j] == BoardState::WHITE) {
                boardMsg += "W";
            }
        }
    }
    send(clientSocket, boardMsg.c_str(), boardMsg.size(), 0);
}

void sendGameOverStatus(int clientSocket1, int clientSocket2, GameResult result) {
    if (result == GameResult::TIE) {
        send(clientSocket1, "Tie", 3, 0);
        send(clientSocket2, "Tie", 3, 0);
    } else if (result == GameResult::PLAYER1_WIN) {
        send(clientSocket1, "Win", 3, 0);
        send(clientSocket2, "Lose", 4, 0);
    } else if (result == GameResult::PLAYER2_WIN) {
        send(clientSocket1, "Lose", 4, 0);
        send(clientSocket2, "Win", 3, 0);
    }
}
bool isBoardFull(const std::vector<std::vector<BoardState>>& board) {
    for (const auto& row : board) {
        for (const auto& state : row) {
            if (state == BoardState::EMPTY) {
                return false; // 비어있는 칸이 있으면 false 반환
            }
        }
    }
    return true; // 모든 칸이 채워져 있으면 true 반환
}

void playGame(Board& gameBoard, int clientSocket1, int clientSocket2)
{
    while(true)
    {
        // 플레이어 1에게 보드 상태 전송
        sendBoardState(clientSocket1, gameBoard);

        // 플레이어 1의 돌 위치 수신 및 처리
        int player1Row, player1Col;
        recv(clientSocket1, &player1Row, sizeof(int), 0);
        recv(clientSocket1, &player1Col, sizeof(int), 0);
        
        if ((player1Row != -100) && (player1Col != -100)) {
                // 게임 보드 출력
                gameBoard.placeStone(player1Row, player1Col, BoardState::BLACK);
                std::cout << "Player 1 (Black) moved: (" << player1Row << ", " << player1Col << ")" << std::endl;

                // 게임 종료 조건 확인
                if (isGameOver(gameBoard.getBoard(), player1Row, player1Col, BoardState::BLACK)) {
                        sendGameOverStatus(clientSocket1, clientSocket2, GameResult::PLAYER1_WIN);
                        break;
                }

        } else {
                std::cout << "Player 1 (Black)'s turn is OVER." << std::endl;
        }


        // 플레이어 2에게 보드 상태 전송
        sendBoardState(clientSocket2, gameBoard);

        // 플레이어 2의 돌 위치 수신 및 처리
        int player2Row, player2Col;
        recv(clientSocket2, &player2Row, sizeof(int), 0);
        recv(clientSocket2, &player2Col, sizeof(int), 0);
        
        if ((player2Row != -100) && (player2Col != -100)) {
                // 게임 보드 출력
                gameBoard.placeStone(player2Row, player2Col, BoardState::WHITE);
                std::cout << "Player 2 (White) moved: (" << player2Row << ", " << player2Col << ")" << std::endl;

                // 게임 종료 조건 확인
                if (isGameOver(gameBoard.getBoard(), player2Row, player2Col, BoardState::WHITE)) {
                        sendGameOverStatus(clientSocket1, clientSocket2, GameResult::PLAYER2_WIN);
                        break;
                }

        } else {
                std::cout << "Player 2 (White)'s turn is OVER." << std::endl;
        }


        // 게임 보드가 가득 차 있는지 확인
        if (isBoardFull(gameBoard.getBoard())) {
                // 게임 종료 처리 (무승부)
                sendGameOverStatus(clientSocket1, clientSocket2, GameResult::TIE);
                break;
        }
    }
}


int main(int argc, char *argv[]) {
    int serverSocket, clientSocket1, clientSocket2;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    if (argc != 2) {
                printf("Usage : %s <port>\n", argv[0]);
                exit(1);
        }

    // 서버 소켓 생성
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create server socket." << std::endl;
        return 1;
    }

    // 서버 주소 설정
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[1]));
    serverAddr.sin_addr.s_addr = INADDR_ANY;

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

    std::cout << "Waiting for two clients to connect..." << std::endl;

    
    // 게임 시작
    while (true) {
        Board board;
        
        // 첫 번째 클라이언트 연결 수락
        clientSocket1 = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket1 < 0) {
                std::cerr << "Failed to accept client connection." << std::endl;
                return 1;
        }
        else{
                std::cout << "Player 1 connected." << std::endl;
                clientSocket2 = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
                if (clientSocket2 < 0) {
                        std::cerr << "Failed to accept client connection." << std::endl;
                        return 1;
                }
                else{
                        std::cout << "Player 2 connected." << std::endl;
                        // 게임 보드 생성
                        
                        playGame(board, clientSocket1, clientSocket2);
                        // 연결 종료
                        close(clientSocket1);
                        close(clientSocket2);
                }
        }
    }
    
    close(serverSocket);

    return 0;
}
