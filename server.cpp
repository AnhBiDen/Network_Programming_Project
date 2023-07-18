#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <map>
#include <fstream>

#define MAX_CLIENT 10
#define RECEIVE_BUFFER_SIZE 1024
#define CREATE_ROOM_CODE "CREATE_ROOM"
#define LEAVE_ROOM_CODE "LEAVE_ROOM"
#define JOIN_ROOM_CODE "JOIN_ROOM"
#define START_GAME_CODE "START_GAME"
#define MOVE_CODE "MOVE"
#define READY_CODE "READY"
#define RESIGN_CODE "RESIGN"
#define LOGIN_CODE "LOGIN"
#define NEW_OWNER_CODE "NEW_OWNER"
#define NEW_GUEST_CODE "NEW_GUEST"

class Room
{
public:
    Room();
    ~Room();

    // ACCESSOR
    std::string getRoomCode() { return roomCode; };
    bool isRoomOccupied() { return isOccupied; };
    bool isOwner(std::thread::id player) { return player == owner; };
    bool isGuest(std::thread::id player) { return player == guest; };
    bool isRoomFull() { return guest != std::thread::id(); };
    bool getNewGuestState() { return newGuest; };
    bool getNewOwnerState() { return newOwner; };
    bool getGameStarting() { return isGameStarting; };
    bool getOwnerSide() { return ownerSide; };
    std::string getOwnerName() { return ownerName; };
    std::string getGuestName() { return guestName; };
    int getOwnerMoveFrom() { return ownerMoveFrom; };
    int getOwnerMoveTo() { return ownerMoveTo; };
    int getGuestMoveFrom() { return guestMoveFrom; };
    int getGuestMoveTo() { return guestMoveTo; };
    bool availableGuestMove() { return guestMoveFrom != -1; };
    bool availableOwnerMove() { return ownerMoveFrom != -1; };
    bool getGuestReady() { return isGuestReady; };
    bool getNewGuestReady() { return newGuestReady; };
    bool getOwnerResign() { return ownerResign; };
    bool getGuestResign() { return guestResign; };

    // MUTATOR
    void setRoomOccupied(bool b) { isOccupied = b; };
    void setRoomCode(std::string roomCode) { this->roomCode = roomCode; };
    void setOwner(std::thread::id owner, std::string ownerName);
    void setGuest(std::thread::id guest, std::string guestName);
    void setNewGuestState(bool state) { newGuest = state; };
    void setNewOwnerState(bool state) { newOwner = state; };
    void setGameStarting(bool state) { isGameStarting = state; };
    void setOwnerSide(bool side) { ownerSide = side; };
    void setGuestMove(int moveFrom, int moveTo)
    {
        guestMoveFrom = moveFrom;
        guestMoveTo = moveTo;
    };
    void setOwnerMove(int moveFrom, int moveTo)
    {
        ownerMoveFrom = moveFrom;
        ownerMoveTo = moveTo;
    };
    void removeOwner();
    void removeGuest();
    void setGuestReady(bool isGuestReady) { this->isGuestReady = isGuestReady; };
    void setNewGuestReady(bool newGuestReady) { this->newGuestReady = newGuestReady; };
    void setGuestResign(bool guestResign) { this->guestResign = guestResign; };
    void setOwnerResign(bool ownerResign) { this->ownerResign = ownerResign; };

private:
    std::string roomCode;
    std::thread::id owner;
    std::string ownerName = "";
    std::thread::id guest;
    std::string guestName = "";
    int ownerMoveFrom = -1;
    int ownerMoveTo = -1;
    int guestMoveFrom = -1;
    int guestMoveTo = -1;
    bool ownerSide;
    bool isOccupied;
    bool isGameStarting;
    bool isGuestReady;
    bool ownerResign;
    bool guestResign;
    bool newGuest, newOwner, newGuestReady;
};

Room::Room()
{
    isOccupied = false;
}

Room::~Room()
{
}

void Room::setOwner(std::thread::id owner, std::string ownerName)
{
    this->owner = owner;
    this->ownerName = ownerName;
}

void Room::setGuest(std::thread::id guest, std::string guestName)
{
    this->guest = guest;
    this->guestName = guestName;
    newGuest = true;
}

void Room::removeOwner()
{
    if (guest != std::thread::id())
    {
        owner = guest;
        ownerName.assign(guestName);
        guest = std::thread::id();
        guestName.assign("");
    }
    else
    {
        owner = std::thread::id();
        ownerName.assign("");
        isOccupied = false;
        roomCode = std::string();
    }
    newOwner = true;
}

void Room::removeGuest()
{
    guest = std::thread::id();
    guestName.assign("");
    newGuest = true;
}


class Account
{
public:
    enum Status {logged_out, logged_in, blocked, wrong_password};
    Account();
    Account(std::string username, std::string password, std::string name,std::string status);
    std::string getName() const;
    Status attemptLogin(const std::string &username, const std::string &password);
    static std::string Stringify(Status status);

private:
    bool checkUsername(std::string username) { return this->username == username; };
    bool checkPassword(std::string password) { return this->password == password; };
    Status loginFailed();
    static const int max_failed_attemp = 3;
    std::string username;
    std::string password;
    std::string name;
    Status status;
    int failedAttemp = 0;
};
Account::Account(){
}

Account::Account(std::string username, std::string password, std::string name, std::string status){
  this->username.assign(username);
  this->password.assign(password);
  this->name.assign(name);
//   this->status.assign(status);
  
  if (status == "logged_out")
    this->status = logged_out;
  else if (status == "logged_in")
    this->status = logged_in;
  else if (status == "blocked")
    this->status = blocked;
  failedAttemp = 0;
}

std::string Account::getName() const
{
    return name;
}

Account::Status Account::attemptLogin(const std::string &username, const std::string &password)
{
    if (!checkUsername(username))
        return logged_out;

    if (!checkPassword(password))
    {
        return loginFailed();
    }

    if (status == blocked)
        return blocked;

    status = logged_in;
    return status;
}

Account::Status Account::loginFailed()
{
    failedAttemp++;
    if (failedAttemp >= max_failed_attemp)
    {
        status = blocked;
        return blocked;
    }
    return wrong_password;
}

std::string Account::Stringify(Status status)
{
    switch (status)
    {
    case logged_out:
        return "logged_out";
        break;
    case logged_in:
        return "logged_in";
        break;
    case blocked:
        return "blocked";
        break;
    case wrong_password:
        return "wrong_password";
        break;
    default:
        return "";
        break;
    }
}

class ServerSocket : public Account
{
public:
    ServerSocket();
    ~ServerSocket();
    void close();
    void handleClient(int client_socket);
    void handleCreateRoomSignal(int client_socket, std::stringstream &ss, int &roomIndex, std::thread::id &threadId);
    void handleJoinRoomSignal(int client_socket, std::stringstream &ss, int &roomIndex, std::thread::id &threadId);
    void handleLeaveRoomSignal(int client_socket, std::stringstream &ss, int &roomIndex, std::thread::id &threadId);
    void handleLoginSignal(int client_socket, std::stringstream &ss);
    void handleReadySignal(int client_socket, std::stringstream &ss, int &roomIndex);
    void handleStartGameSignal(int client_socket, std::stringstream &ss, int &roomIndex);
    void handleMoveSignal(int client_socket, std::stringstream &ss, int &roomIndex, std::thread::id &threadId);
    void handleResignSignal(int client_socket, std::stringstream &ss, int &roomIndex, std::thread::id &threadId);
    void handleRoomStatus(int client_socket, int &roomIndex, std::thread::id &threadId);
    void initializeAccountList();
    void error(std::string message);
    void error(std::string message, bool isThread);

    std::string gen_random(const int len);
    int coin_flip();
    int getServerSocket() const
    {
        return server_so;
    }

private:
    int server_so;
    std::map<std::string, Account> accountList;
    Room roomList[MAX_CLIENT];
};

ServerSocket::ServerSocket()
{
    if ((server_so = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        error("Failed to create socket.\n");

    std::cout << "Socket created.\n";

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(5500);

    if (bind(server_so, (struct sockaddr *)&server, sizeof(server)) == -1)
        error("Bind failed.\n");

    if (listen(server_so, MAX_CLIENT) < 0)
        error("Listen failed.\n");

    initializeAccountList();

    std::cout << "Waiting for incoming connections..\n";
}

ServerSocket::~ServerSocket()
{
    close();
}

void ServerSocket::close()
{
    ::close(server_so);
    return;
}

void ServerSocket::error(std::string message)
{
    std::cerr << message << ": " << strerror(errno) << "\n";
    exit(1);
}

void ServerSocket::error(std::string message, bool isThread)
{
    std::cerr << message << ": " << strerror(errno) << "\n";
    if (!isThread)
        exit(1);
}

void ServerSocket::handleClient(int client_socket)
{
    char buffer[RECEIVE_BUFFER_SIZE];
    std::string message;
    std::string code;
    int roomIndex = -1;
    int bytes_received;
    std::thread::id threadId = std::this_thread::get_id();

    while (1)
    {
        bytes_received = recv(client_socket, buffer, RECEIVE_BUFFER_SIZE, 0);

        if (bytes_received > 0)
        {
            std::stringstream ss(buffer);
            std::string token;
            std::getline(ss, token, '\n');
            std::cout << "Token: " << token << '\n';

            if (!token.compare(CREATE_ROOM_CODE))
            {
                handleCreateRoomSignal(client_socket, ss, roomIndex, threadId);
            }
            else if (!token.compare(LEAVE_ROOM_CODE))
            {
                handleLeaveRoomSignal(client_socket, ss, roomIndex, threadId);
                // Close the client socket
                ::close(client_socket);
                break;
            }
            else if (!token.compare(JOIN_ROOM_CODE))
            {
                handleJoinRoomSignal(client_socket, ss, roomIndex, threadId);
            }
            else if (!token.compare(START_GAME_CODE))
            {
                handleStartGameSignal(client_socket, ss, roomIndex);
            }
            else if (!token.compare(MOVE_CODE))
            {
                handleMoveSignal(client_socket, ss, roomIndex, threadId);
            }
            else if (!token.compare(READY_CODE))
            {
                handleReadySignal(client_socket, ss, roomIndex);
            }
            else if (!token.compare(RESIGN_CODE))
            {
                handleResignSignal(client_socket, ss, roomIndex, threadId);
                // Close the client socket
                ::close(client_socket);
                break;
            }
            else if (!token.compare(LOGIN_CODE))
            {
                handleLoginSignal(client_socket, ss);
            }
            // Reset the buffer
            memset(buffer, 0, RECEIVE_BUFFER_SIZE);
        }
        else if (bytes_received == 0)
        {
            // Connection closed by the client
            std::cout << "Connection closed by the client: " << client_socket << std::endl;
            break;
        }
        else
        {
            if (errno != EWOULDBLOCK)
            {
                std::cerr << "Failed to receive data. Error code: " << strerror(errno) << std::endl;
                break;
            }
            // No data available in the receive buffer
            // Do other work or sleep for a while before calling recv() again
        }

        if (roomIndex >= 0)
        {
            handleRoomStatus(client_socket, roomIndex, threadId);
        }
    }

    std::cout << "Closing client socket.." << std::endl;
    ::close(client_socket);
}

void ServerSocket::handleCreateRoomSignal(int client_socket, std::stringstream &ss, int &roomIndex, std::thread::id &threadId)
{
    std::string token, code, message;

    std::getline(ss, token, '\n');
    std::cout << "Create room with owner: " << token << '\n';
    std::cout << "Creating room\n";

    std::string roomCode = gen_random(6);
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (!roomList[i].isRoomOccupied())
        {
            roomIndex = i;
            roomList[i].setRoomCode(roomCode);
            roomList[i].setRoomOccupied(true);
            roomList[i].setOwner(threadId, token);
            code.assign(CREATE_ROOM_CODE);
            message = code + '\n' + roomCode + '\n';
            send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
            std::cout << "Room " << i << " is occupied with code " << roomCode << '\n';
            break;
        }
    }
}

void ServerSocket::handleJoinRoomSignal(int client_socket, std::stringstream &ss, int &roomIndex, std::thread::id &threadId)
{
    std::string token, code, message;

    bool foundRoom = false;
    bool roomFull = false;
    std::getline(ss, token, '\n');
    std::cout << "Find room with code: " << token << '\n';

    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (roomList[i].isRoomOccupied())
        {
            if (roomList[i].getRoomCode() == token)
            {
                foundRoom = true;
                if (roomList[i].isRoomFull())
                {
                    roomFull = true;
                    break;
                }
                std::getline(ss, token, '\n');
                std::cout << "Player: " << token << " is joining room\n";
                roomIndex = i;
                roomList[i].setGuest(threadId, token);
                code.assign(JOIN_ROOM_CODE);
                message = code + '\n' + "1\n" + roomList[i].getOwnerName() + '\n';
                send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
                std::cout << "Room " << i << " with code " << roomList[i].getRoomCode() << " has guest joined\n";
                break;
            }
        }
    }
    if (!foundRoom)
    {
        code.assign(JOIN_ROOM_CODE);
        message = code + '\n' + "0\n";
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        std::cout << "Room with code " << token << " cannot be found\n";
    }
    else if (roomFull)
    {
        code.assign(JOIN_ROOM_CODE);
        message = code + '\n' + "2\n";
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        std::cout << "Room with code " << token << " is full\n";
    }
}

void ServerSocket::handleLeaveRoomSignal(int client_socket, std::stringstream &ss, int &roomIndex, std::thread::id &threadId)
{
    std::string token, code, message;
    if (roomIndex < 0)
    {
        std::cout << "Player not in a room but sends leave room request. Ignoring...\n";
        return;
    }

    if (roomList[roomIndex].isOwner(threadId))
    {
        std::cout << "Room " << roomIndex << " with code " << roomList[roomIndex].getRoomCode() << " has lost its owner\n";
        roomList[roomIndex].removeOwner();
        roomIndex = -1;
        code.assign(LEAVE_ROOM_CODE);
        message = code + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
    }
    else if (roomList[roomIndex].isGuest(std::this_thread::get_id()))
    {
        std::cout << "Room " << roomIndex << " with code " << roomList[roomIndex].getRoomCode() << " has lost its guest\n";
        roomList[roomIndex].removeGuest();
        roomIndex = -1;
        code.assign(LEAVE_ROOM_CODE);
        message = code + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
    }
    else
    {
        std::cout << "Something is wrong, can't find room to leave\n";
    }
}

void ServerSocket::handleLoginSignal(int client_socket, std::stringstream &ss)
{
    std::string username, password, token, code, message;

    std::getline(ss, token, '\n');
    std::cout << token << '\n';
    username = token;
    std::getline(ss, token, '\n');
    std::cout << token << '\n';
    password = token;
    // Find account logic here
    Account::Status status;
    std::string name;
    std::string statusMessage;
    for (auto &account : accountList)
    {
        status = account.second.attemptLogin(username, password);
        if (status != Account::Status::logged_out)
        {
            name = account.second.getName();
            break;
        }
    }
    code.assign(LOGIN_CODE);
    message = code + '\n' + Account::Stringify(status) + '\n' + name + '\n';
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
}

void ServerSocket::handleReadySignal(int client_socket, std::stringstream &ss, int &roomIndex)
{
    std::string token, code, message;

    std::getline(ss, token, '\n');
    std::cout << token << '\n';
    roomList[roomIndex].setGuestReady(std::stoi(token));
    roomList[roomIndex].setNewGuestReady(true);

    code.assign(READY_CODE);
    message = code + '\n' + token + '\n';
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
}

void ServerSocket::handleStartGameSignal(int client_socket, std::stringstream &ss, int &roomIndex)
{
    std::string token, code, message;

    if (roomIndex < 0)
    {
        std::cout << "Player not in a room but sends start game request. Ignoring...\n";
        return;
    }
    code.assign(START_GAME_CODE);
    int newInt = coin_flip();
    std::cout << "coin: " << newInt << '\n';
    std::string playerSide = newInt ? "white" : "black";
    message = code + '\n' + playerSide + '\n';
    roomList[roomIndex].setGameStarting(true);
    roomList[roomIndex].setOwnerSide(newInt);
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
}

void ServerSocket::handleMoveSignal(int client_socket, std::stringstream &ss, int &roomIndex, std::thread::id &threadId)
{
    std::string token, code, message;

    int moveFrom, moveTo;
    std::getline(ss, token, '\n');
    std::cout << token << '\n';
    moveFrom = std::stoi(token);
    std::getline(ss, token, '\n');
    std::cout << token << '\n';
    moveTo = std::stoi(token);
    if (roomList[roomIndex].isOwner(threadId))
    {
        roomList[roomIndex].setOwnerMove(moveFrom, moveTo);
    }
    else if (roomList[roomIndex].isGuest(threadId))
    {
        roomList[roomIndex].setGuestMove(moveFrom, moveTo);
    }
    else
    {
        std::cout << "Can't find player to set move\n";
    }

    code.assign(MOVE_CODE);
    message = code + '\n' + std::to_string(moveFrom) + '\n' + std::to_string(moveTo) + '\n';
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
}

void ServerSocket::handleResignSignal(int client_socket, std::stringstream &ss, int &roomIndex, std::thread::id &threadId)
{
    std::string token, code, message;
    if (roomIndex < 0)
    {
        std::cout << "Player not in a room but sends resign request. Ignoring...\n";
        return;
    }

    if (roomList[roomIndex].isOwner(threadId))
    {
        roomList[roomIndex].setOwnerResign(true);
        roomList[roomIndex].setOwnerMove(-1, -1);
    }
    else if (roomList[roomIndex].isGuest(threadId))
    {
        roomList[roomIndex].setGuestResign(true);
        roomList[roomIndex].setGuestMove(-1, -1);
    }
    else
    {
        std::cout << "Can't find player to set move\n";
    }
    code.assign(RESIGN_CODE);
    message = code + '\n';
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
}

void ServerSocket::handleRoomStatus(int client_socket, int &roomIndex, std::thread::id &threadId)
{
    std::string token, code, message;

    if (roomList[roomIndex].getNewGuestState())
    {
        code.assign(NEW_GUEST_CODE);
        message = code + '\n' + roomList[roomIndex].getGuestName() + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        roomList[roomIndex].setNewGuestState(false);
    }
    if (roomList[roomIndex].getNewOwnerState())
    {
        code.assign(NEW_OWNER_CODE);
        message = code + '\n' + roomList[roomIndex].getOwnerName() + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        roomList[roomIndex].setNewOwnerState(false);
    }
    if (roomList[roomIndex].getGameStarting())
    {
        code.assign(START_GAME_CODE);
        std::string playerSide = roomList[roomIndex].getOwnerSide() ? "white" : "black";
        message = code + '\n' + playerSide + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        roomList[roomIndex].setGameStarting(false);
    }
    if (roomList[roomIndex].availableOwnerMove())
    {
        code.assign(MOVE_CODE);
        message = code + '\n' + std::to_string(roomList[roomIndex].getOwnerMoveFrom()) + '\n' + std::to_string(roomList[roomIndex].getOwnerMoveTo()) + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        roomList[roomIndex].setOwnerMove(-1, -1);
    }
    if (roomList[roomIndex].availableGuestMove())
    {
        code.assign(MOVE_CODE);
        message = code + '\n' + std::to_string(roomList[roomIndex].getGuestMoveFrom()) + '\n' + std::to_string(roomList[roomIndex].getGuestMoveTo()) + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        roomList[roomIndex].setGuestMove(-1, -1);
    }
    if (roomList[roomIndex].getNewGuestReady())
    {
        code.assign(READY_CODE);
        message = code + '\n' + std::to_string(roomList[roomIndex].getGuestReady()) + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        roomList[roomIndex].setNewGuestReady(false);
    }
    if (roomList[roomIndex].getOwnerResign())
    {
        code.assign(RESIGN_CODE);
        message = code + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        roomList[roomIndex].setOwnerResign(false);
    }
    if (roomList[roomIndex].getGuestResign())
    {
        code.assign(RESIGN_CODE);
        message = code + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        roomList[roomIndex].setGuestResign(false);
    }
}

void ServerSocket::initializeAccountList()
{
    std::ifstream inputFile("users.txt");

    if (!inputFile)
    {
        std::cout << "Error opening the file." << std::endl;
        exit(1);
    }

    std::string line;
    std::string username;
    std::string password;
    std::string name;
    std::string status;
    int lineCount = 0;
    while (std::getline(inputFile, line))
    {
        switch (lineCount % 4)
        {
        case 0:
            username = line;
            break;
        case 1:
            password = line;
            break;
        case 2:
            name = line;
            break;
        case 3:
            status = line;
            accountList.emplace(username, Account(username, password, name, status) );
            break;
        }
        lineCount++;
    }
}

std::string ServerSocket::gen_random(const int len)
{
    std::string tmp_s;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    srand(time(0));

    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
    {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

int ServerSocket::coin_flip()
{
    srand(time(0));
    return rand() % 2;
}


void handle_client(int client_socket);
std::string gen_random(const int len);
std::map<std::string, Account> initializeAccountList();
void displayAccountList(const std::map<std::string, Account>& accountList);

int main()
{
    ServerSocket server_socket;
    int server_so = server_socket.getServerSocket();
    struct sockaddr_in client_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);
    int client_so[MAX_CLIENT];
    std::map<int, int> connectedClients; // std::pair<client_socket, roomIndex>

    for (int i = 0; i < MAX_CLIENT; i++)
    {
        client_so[i] = 0;
    }

    fd_set readfds;
    fd_set writefds;
    fd_set exceptfds;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);

        FD_SET(server_so, &readfds);

        int max_fd = server_so;

        for (int i = 0; i < MAX_CLIENT; i++)
        {
            int client_socket = client_so[i];

            if (client_socket > 0)
            {
                FD_SET(client_socket, &readfds);
                FD_SET(client_socket, &writefds);
                FD_SET(client_socket, &exceptfds);
            }

            if (client_socket > max_fd)
            {
                max_fd = client_socket;
            }
        }

        // Check if there are any sockets ready for I/O operations
        int activity = select(max_fd + 1, &readfds, &writefds, &exceptfds, NULL);

        // Check for closed connections
        for (auto it = connectedClients.begin(); it != connectedClients.end();)
        {
            int client_socket = it->first;
            int roomIndex = it->second;

            if (FD_ISSET(client_socket, &exceptfds))
            {
                // Connection closed by the client
                std::cout << "Connection closed by the client: " << client_socket << std::endl;
                roomIndex = -1;

                // Close the socket
                ::close(client_socket);

                // Remove the client from the connectedClients array
                it = connectedClients.erase(it);
            }
            else
            {
                ++it;
            }
        }

        if ((activity < 0) && (errno != EINTR))
        {
            server_socket.error("Select error");
        }

        // Check if there is a new connection
        if (FD_ISSET(server_so, &readfds))
        {
            int new_socket = accept(server_so, (struct sockaddr *)&client_addr, &sin_size);

            if (new_socket < 0)
            {
                server_socket.error("Accept failed");
            }

            std::cout << "New connection, socket fd: " << new_socket << ", ip: " << inet_ntoa(client_addr.sin_addr) << ", port: " << ntohs(client_addr.sin_port) << std::endl;

            // Add new socket to array of client sockets
            for (int i = 0; i < MAX_CLIENT; i++)
            {
                if (client_so[i] == 0)
                {
                    client_so[i] = new_socket;
                    std::cout << "Adding to list of sockets as " << i << std::endl;
                    break;
                }
            }
        }

        // Check for I/O operations on existing sockets
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            int client_socket = client_so[i];
            int roomIndex = connectedClients[client_socket];
            if (client_socket > 0 && FD_ISSET(client_socket, &readfds))
            {
                server_socket.handleClient(client_socket);
                // Update the roomIndex in the connectedClients map
                connectedClients[client_socket] = roomIndex;
            }

            // Check if the client socket was closed
            if (client_socket > 0 && FD_ISSET(client_socket, &exceptfds))
            {
                std::cout << "Client socket closed: " << client_socket << std::endl;
                // Close the client socket
                ::close(client_socket);
                // Remove the client socket from the client_so array
                client_so[i] = -1;
                // Remove the client socket from the connectedClients map
                connectedClients.erase(client_socket);
            }
        }
    }

    server_socket.close();

    return 0;
}
