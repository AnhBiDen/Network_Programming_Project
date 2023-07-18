#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <map>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

const int RECEIVE_BUFFER_SIZE = 4096;
#define JOIN_ROOM_CODE "JOIN"
#define CREATE_ROOM_CODE "CREATE"
#define LEAVE_ROOM_CODE "LEAVE"
#define START_GAME_CODE "START"
#define NEW_GUEST_CODE "NEWGUEST"
#define NEW_OWNER_CODE "NEWOWNER"
#define MOVE_CODE "MOVE"
#define READY_CODE "READY"
#define RESIGN_CODE "RESIGN"
#define LOGIN_CODE "LOGIN"


class ClientSocket
{
public:
  ClientSocket(std::string server_ip, unsigned short server_port);
  int GetClientSocket() { return so; };

  void error(std::string message);
  void close();
  void handleBufferRead();

  // ACCESSOR
  std::string getRoomCode() { return roomCode; };
  std::string getPlayerName() { return name; };
  std::string getOpponentName() { return opponentName; };
  std::string getLoginStatus() { return loginStatus; };
  bool getRoomFoundState() { return roomNotFound; };
  bool getRoomFull() { return roomFull; };
  bool getIsOwner() { return isOwner; };
  bool getIsGuestReady() { return isGuestReady; };
  // MUTATOR
  void setRoomCode(std::string newRoomCode);
//   void setDisplayPtr(Display *display);
//   void setBoardPtr(Board *board);
  void setRoomFoundState(bool b) { roomNotFound = b; };
  void setGuestReady(bool isGuestReady) { this->isGuestReady = isGuestReady; };
  void setUsername(std::string username) { this->username = username; };
  void setPassword(std::string password) { this->password = password; };

  void sendLoginSignal();
  void handleLoginSignal(std::string reply, std::string name);
  void sendJoinRoom();
  void handleJoinRoom(std::string reply, std::string opponentName);
  void sendCreateRoom();
  void handleCreateRoom(std::string roomCode);
  void sendLeaveRoom();
  void handleLeaveRoom();
  void sendStartGame();
  void handleStartGame(std::string side);
//   void sendMoveSignal(int moveFrom, int moveTo);
//   void handleMoveSignal(int moveFrom, int moveTo);
  void sendReadySignal();
  void handleReadySignal(int readyStatus);
  void sendResignSignal();
  void handleResignSignal();
  void handleNewGuest(std::string newGuestName);
  void handleNewOwner(std::string newGuestName);
  bool isRoomFull() { return opponentName != ""; };

private:
  int so;
  std::string roomCode = "";
  std::string username = "";
  std::string password = "";
  std::string name = "";
  std::string opponentName = "";
  std::string loginStatus;
  bool roomNotFound;
  bool roomFull;
  bool isOwner;
  bool isGuestReady;
//   Display *displayPtr;
//   Board *boardPtr;
};

std::string gen_random(const int len)
{
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::string tmp_s;
  tmp_s.reserve(len);

  srand(time(0));
  for (int i = 0; i < len; ++i)
  {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return tmp_s;
}

void ClientSocket::error(std::string message)
{
  std::cerr << message << ": " << strerror(errno) << "\n";
  exit(1);
  return;
}

void ClientSocket::close()
{
  ::close(this->so);
  return;
}

void ClientSocket::setRoomCode(std::string newRoomCode)
{
  roomCode = newRoomCode;
}


ClientSocket::ClientSocket(std::string server_ip, unsigned short server_port)
{
  struct sockaddr_in server;

  if ((so = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    error("Error creating the socket");

  std::cout << "Socket created.\n";

  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(server_ip.c_str());
  server.sin_port = htons(server_port);

  if (connect(so, (struct sockaddr *)&server, sizeof(server)) < 0)
    error("Failed to connect");

  std::cout << "Connected to " << server_ip << ":" << server_port << "\n";

  fcntl(so, F_SETFL, O_NONBLOCK);

  name.assign(gen_random(3)); // assign name here
  roomNotFound = false;
  roomFull = false;
  isOwner = false;
  isGuestReady = false;
}

void ClientSocket::sendLoginSignal()
{
  std::string message;
  std::string code;

  code.assign(LOGIN_CODE);
  message = code + '\n' + username + '\n' + password + '\n';
  if (send(so, message.c_str(), message.length(), 0) == -1)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleLoginSignal(std::string reply, std::string name)
{
  loginStatus = reply;
  if (reply == "logged_in")
  {
    // displayPtr->setMenu(Display::Menu::mainMenu);
    this->name = name;
  }
}

void ClientSocket::sendJoinRoom()
{
  std::string message;
  std::string code;

  code.assign(JOIN_ROOM_CODE);
  message = code + '\n' + roomCode + '\n' + name + '\n';
  if (send(so, message.c_str(), message.length(), 0) == -1)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleJoinRoom(std::string reply, std::string opponentName)
{
  roomNotFound = false;
  roomFull = false;
  if (reply == "1")
  {
    std::cout << "Room found\n";
    // displayPtr->setMenu(Display::Menu::roomMenu);
    this->opponentName.assign(opponentName);
  }
  else if (reply == "0")
  {
    std::cout << "Room not found\n";
    roomNotFound = true;
  }
  else if (reply == "2")
  {
    std::cout << "Room is full\n";
    roomFull = true;
  }
  else
  {
    std::cout << "Message not recognized. Exiting\n";
    exit(0);
  }
}

void ClientSocket::sendCreateRoom()
{
  std::string message;
  std::string code;

  std::cout << "Sending create room message\n";
  roomCode.assign("");
  code.assign(CREATE_ROOM_CODE);
  message = code + '\n' + name + '\n';
  if (send(so, message.c_str(), message.length(), 0) == -1)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleCreateRoom(std::string roomCode)
{
  this->roomCode.assign(roomCode);
  isOwner = true;
}

void ClientSocket::sendLeaveRoom()
{
  std::string code;
  std::string message;

  std::cout << "Sending leaving message\n";
  code.assign(LEAVE_ROOM_CODE);
  message = code + '\n';
  if (send(so, message.c_str(), message.length(), 0) == -1)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleLeaveRoom()
{
  std::cout << "Leaved room\n";
  opponentName = "";
  roomCode = "";
  isOwner = false;
  isGuestReady = false;
}

void ClientSocket::sendStartGame()
{
  std::string code;
  std::string message;

  std::cout << "Sending start game signal\n";
  code.assign(START_GAME_CODE);
  message = code + '\n';
  if (send(so, message.c_str(), message.length(), 0) == -1)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleStartGame(std::string side)
{
  
}

void ClientSocket::handleNewGuest(std::string newGuestName)
{
  opponentName.assign(newGuestName);
  if (newGuestName == "")
  {
    isGuestReady = false;
  }
}

void ClientSocket::handleNewOwner(std::string newOwnerName)
{
  if (newOwnerName == name)
  {
    opponentName.assign("");
    isOwner = true;
  }
  else
  {
    opponentName.assign(newOwnerName);
    isOwner = false;
  }
}

void ClientSocket::sendReadySignal()
{
  std::string code;
  std::string message;

  std::cout << "Sending ready signal\n";
  code.assign(READY_CODE);
  message = code + '\n' + (isGuestReady ? std::to_string(0) : std::to_string(1)) + '\n';
  if (send(so, message.c_str(), message.length(), 0) == -1)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleReadySignal(int readyStatus)
{
  isGuestReady = readyStatus;
}

void ClientSocket::sendResignSignal()
{
  std::string code;
  std::string message;

  std::cout << "Sending resign signal\n";
  code.assign(RESIGN_CODE);
  message = code + '\n';
  if (send(so, message.c_str(), message.length(), 0) == -1)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleResignSignal()
{
 
}

void ClientSocket::handleBufferRead()
{
  int bytes_received = 0;
  char buffer[RECEIVE_BUFFER_SIZE];
  std::string token;

  do
  {
    bytes_received = recv(so, buffer, RECEIVE_BUFFER_SIZE, 0);
    if (bytes_received > 0)
    {
      // Process the received data
      std::stringstream ss(buffer);
      std::getline(ss, token, '\n');
      std::cout << "Token: " << token << '\n';
      if (token == CREATE_ROOM_CODE)
      {
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        handleCreateRoom(token);
      }
      else if (token == LEAVE_ROOM_CODE)
      {
        handleLeaveRoom();
      }
      else if (token == JOIN_ROOM_CODE)
      {
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        std::string token2;
        std::getline(ss, token2, '\n');
        handleJoinRoom(token, token2);
      }
      else if (token == NEW_GUEST_CODE)
      {
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        handleNewGuest(token);
      }
      else if (token == NEW_OWNER_CODE)
      {
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        handleNewOwner(token);
      }
      else if (token == START_GAME_CODE)
      {
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        handleStartGame(token);
      }
      else if (token == MOVE_CODE)
      {
        int moveFrom, moveTo;
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        moveFrom = std::stoi(token);
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        moveTo = std::stoi(token);
        // handleMoveSignal(moveFrom, moveTo);
      }
      else if (token == READY_CODE)
      {
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        handleReadySignal(std::stoi(token));
      }
      else if (token == RESIGN_CODE)
      {
        handleResignSignal();
      }
      else if (token == LOGIN_CODE)
      {
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        std::string reply = token;
        std::getline(ss, token, '\n');
        std::cout << "Token: " << token << '\n';
        handleLoginSignal(reply, token);
      }
    }
    else if (bytes_received == 0)
    {
      ClientSocket::error("Connection closed by the server");
      break;
    }
    else
    {
      if (errno != EWOULDBLOCK)
      {
        ClientSocket::error("Failed to receive data. Error code");
        break;
      }
      // No data available in the receive buffer
      // Do other work or sleep for a while before calling recv() again
    }
  } while (bytes_received > 0);
}

bool quit = false, muted = true, start = false, isTyping = false;
std::string inputText = "";
using namespace std;

int main() {
    ClientSocket clientSocket("127.0.0.1", 5500);
    std::string input;

    while (true) {
        std::cout << "1. Đăng nhập\n";
        std::cout << "2. Thoát\n";
        std::cout << "Chọn một lựa chọn: ";
        std::getline(std::cin, input);

        if (input == "1") {
            std::string username, password;
            std::cout << "Nhập tên người dùng: ";
            std::getline(std::cin, username);
            std::cout << "Nhập mật khẩu: ";
            std::getline(std::cin, password);

            // Gửi thông tin đăng nhập tới server
            clientSocket.setUsername(username);
            clientSocket.setPassword(password);
            clientSocket.sendLoginSignal();
        } else if (input == "2") {
            // Thoát khỏi chương trình
            break;
        } else {
            std::cout << "Lựa chọn không hợp lệ. Vui lòng chọn lại.\n";
        }

        clientSocket.handleBufferRead();

        // Kiểm tra trạng thái đăng nhập
        std::string loginStatus = clientSocket.getLoginStatus();
        if (loginStatus == "logged_in") {
            while (true) {
                std::cout << "1. Tạo phòng\n";
                std::cout << "2. Tham gia phòng\n";
                std::cout << "3. Thoát (Đăng xuất)\n";
                std::cout << "Chọn một lựa chọn: ";
                std::getline(std::cin, input);

                if (input == "1") {
                    // Gửi yêu cầu tạo phòng tới server
                    clientSocket.sendCreateRoom();
                    clientSocket.handleBufferRead();
                    std::string roomCode = clientSocket.getRoomCode();
                    std::cout << "Mã phòng của bạn: " << roomCode << "\n";
                } else if (input == "2") {
                    std::string roomCode;
                    std::cout << "Nhập mã phòng: ";
                    std::getline(std::cin, roomCode);

                    // Gửi yêu cầu tham gia phòng tới server
                    clientSocket.setRoomCode(roomCode);
                    clientSocket.sendJoinRoom();
                    clientSocket.handleBufferRead();

                    if (!clientSocket.getRoomFoundState()) {
                        std::cout << "Phòng không tồn tại.\n";
                        continue;
                    }

                    if (clientSocket.getRoomFull()) {
                        std::cout << "Phòng đã đầy.\n";
                        continue;
                    }

                    std::cout << "Đã tham gia phòng.\n";
                } else if (input == "3") {
                    // Gửi yêu cầu đăng xuất tới server
                    clientSocket.sendLeaveRoom();
                    break;
                } else {
                    std::cout << "Lựa chọn không hợp lệ. Vui lòng chọn lại.\n";
                }
            }

            // Đăng xuất thành công, quay lại lựa chọn đăng nhập/ thoát
        } else {
            std::cout << "Đăng nhập không thành công. Vui lòng kiểm tra tên người dùng và mật khẩu.\n";
        }
    }

    return 0;
}
