#include <thread>
#include <queue>
#include <sockpp/tcp_acceptor.h>

int16_t port = 5110;

void handleClient(uint32_t playerId);
void doSyncWithClient(uint32_t playerId);

enum class EActionPacketType
{
    PLAYER_JOINED,
    PLAYER_LEFT,
    CHAT_RECEIVED,
    CHAT_SEND
};

struct ActionPacket
{
    EActionPacketType type;
    char data[512];
};

struct Player
{
    std::string username;
    sockpp::tcp_socket tcpSocket;
    float xPos, yPos;
    std::queue<ActionPacket> actionQueue;
    bool disconnect = false;
};

std::vector<Player*> players;

int main()
{
    std::cout << "Starting server on port " << port << "..." << std::endl;

    sockpp::socket_initializer sockInit;

    sockpp::tcp_acceptor server(port);

    if (!server)
    {
        std::cerr << "Error: " << server.last_error_str() << std::endl;
        return -1;
    }

    std::cout << "Server running" << std::endl;

    while (true)
    {
        sockpp::tcp_socket client = server.accept();

        if (!client)
            std::cerr << "Failed to accept client" << std::endl;
        else
        {
            std::cout << "Client connecting... " << client.peer_address() << std::endl;

            auto player = new Player();
            player->tcpSocket = std::move(client);

            players.push_back(player);
            uint32_t id = players.size()-1;

            std::thread thr(handleClient, id);
            thr.detach();
        }
    }

    return 0;
}

struct NetPlayerUpdate
{
    char username[64];
    float xPos, yPos;
    bool isLast = false;
};

bool sendAndReceive(sockpp::tcp_socket* tcpSocket, const std::string& strToSend, std::string* outAnswer, float timeoutMillisecond = 10000.f)
{
    if (!tcpSocket) return false;
    if (!outAnswer) return false;

    tcpSocket->write_timeout(std::chrono::duration<float, std::milli>(timeoutMillisecond));
    tcpSocket->read_timeout(std::chrono::duration<float, std::milli>(timeoutMillisecond));

    tcpSocket->write(strToSend);

    char buffer[512] = "\0";
    ssize_t n = tcpSocket->read(buffer, sizeof(buffer));

    if (n > 0)
    {
        *outAnswer = buffer;
        return true;
    }

    return false;
}

bool sendVoidAndCheckAnswer(sockpp::tcp_socket* tcpSocket, void* pObj, size_t objSize, const std::string& answer, float timeoutMillisecond = 5000.f)
{
    if (!tcpSocket) return false;

    tcpSocket->write_timeout(std::chrono::duration<float, std::milli>(timeoutMillisecond));
    tcpSocket->read_timeout(std::chrono::duration<float, std::milli>(timeoutMillisecond));

    if (tcpSocket->write(pObj, objSize) < 0)
        return false;

    char buffer[512] = "\0";
    ssize_t n = tcpSocket->read(buffer, sizeof(buffer));

    if (n > 0)
    {
        return (std::string(buffer) == answer);
    }

    return false;
}

bool sendAndCheckAnswer(sockpp::tcp_socket* tcpSocket, const std::string& strToSend, const std::string& answerRequired)
{
    std::string answer;

    if (!sendAndReceive(tcpSocket, strToSend, &answer))
        return false;

    return (answer == answerRequired);
}

void handleClient(uint32_t playerId)
{
    Player* playerPtr = players[playerId];
    if (!playerPtr) return;

    char buffer[512] = "\0";
    ssize_t n = playerPtr->tcpSocket.read(buffer, sizeof(buffer));

    if (std::string(buffer) != "logon")
    {
        std::cout << "Failed to receive logon request from " << playerPtr->tcpSocket.peer_address() << std::endl;
        return;
    }

    playerPtr->tcpSocket.write("ok");

    n = playerPtr->tcpSocket.read(buffer, sizeof(buffer));
    playerPtr->username = buffer;

    std::cout << "Client connected: " << playerPtr->username << std::endl;

    for (size_t i = 0; i < players.size(); ++i)
    {
        if (i != playerId && !players[i]->disconnect)
        {
            ActionPacket ap = {};
            ap.type = EActionPacketType::PLAYER_JOINED;
#ifdef _WIN32
            strcpy_s(ap.data, playerPtr->username.c_str());
#else
            strcpy(ap.data, playerPtr->username.c_str());
#endif
            players[i]->actionQueue.push(ap);

            ActionPacket ap2 = {};
            ap2.type = EActionPacketType::PLAYER_JOINED;
#ifdef _WIN32
            strcpy_s(ap2.data, players[i]->username.c_str());
#else
            strcpy(ap2.data, players[i]->username.c_str());
#endif
            playerPtr->actionQueue.push(ap2);
        }
    }

    while (true)
    {
        doSyncWithClient(playerId);
        //std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(50));

        if (playerPtr->disconnect)
        {
            for (size_t i = 0; i < players.size(); ++i)
            {
                if (!players[i]->disconnect)
                {
                    ActionPacket ap = {};
                    ap.type = EActionPacketType::PLAYER_LEFT;
#ifdef _WIN32
                    strcpy_s(ap.data, playerPtr->username.c_str());
#else
                    strcpy(ap.data, playerPtr->username.c_str());
#endif
                    players[i]->actionQueue.push(ap);
                }
            }

            delete playerPtr;
            //players.erase(players.begin() + playerId);

            break;
        }
    }

    std::cout << "Player disconnected" << std::endl;
}

void doSyncWithClient(uint32_t playerId)
{
    Player* player = players[playerId];
    if (!player) return;

    if (!sendAndCheckAnswer(&player->tcpSocket, "sync.req", "sync.ack"))
    {
        std::cerr << "Player Sync Failed (nack)" << std::endl;
        players[playerId]->disconnect = true;
        return;
    }

    if (!player->actionQueue.empty())
    {
        if (!sendAndCheckAnswer(&player->tcpSocket, "aq.has", "aq.ok"))
        {
            std::cerr << "Player Sync Failed (nok)" << std::endl;
            return;
        }

        if (!sendVoidAndCheckAnswer(&player->tcpSocket, &player->actionQueue.front(), sizeof(ActionPacket), "aq.recv"))
        {
            std::cerr << "Player AQ sync failed (nrecv)" << std::endl;
            return;
        }

        player->actionQueue.pop();
    }
    else
    {
        if (!sendAndCheckAnswer(&player->tcpSocket, "aq.none", "aq.ok"))
        {
            std::cerr << "Player Sync Failed (nok)" << std::endl;
            return;
        }
    }

    if (players.size() > 1)
    {
        if (!sendAndCheckAnswer(&player->tcpSocket, "ppos.begin", "ppos.ok"))
        {
            std::cerr << "Player Sync Failed (nok)" << std::endl;
            return;
        }

        for (size_t i = 0; i < players.size(); ++i)
        {
            if (i != playerId && !players[i]->disconnect)
            {
                NetPlayerUpdate upd = {};
#ifdef _WIN32
                strcpy_s(upd.username, players[i]->username.c_str());
#else
                strcpy(upd.username, players[i]->username.c_str());
#endif
                upd.xPos = players[i]->xPos;
                upd.yPos = players[i]->yPos;

                if (playerId == players.size()-1)
                    upd.isLast = (i == players.size()-2);
                else
                    upd.isLast = (i == players.size()-1);

                if (!sendVoidAndCheckAnswer(&player->tcpSocket, &upd, sizeof(upd), "ppos.recv"))
                {
                    std::cerr << "A Pos update failed (nrecv)" << std::endl;
                    break;
                }
            }
        }

        if (!sendAndCheckAnswer(&player->tcpSocket, "ppos.end", "ppos.end"))
        {
            std::cerr << "Player Sync Failed (nend)" << std::endl;
            return;
        }
    }
    else
    {
        if (!sendAndCheckAnswer(&player->tcpSocket, "ppos.none", "ppos.ok"))
        {
            std::cerr << "Player Sync Failed (nok)" << std::endl;
            return;
        }
    }

    player->tcpSocket.write("pos.req");

    NetPlayerUpdate upd = {};
    player->tcpSocket.read(&upd, sizeof(NetPlayerUpdate));

    players[playerId]->xPos = upd.xPos;
    players[playerId]->yPos = upd.yPos;

    if (!sendAndCheckAnswer(&player->tcpSocket, "pos.recv", "pos.end"))
    {
        std::cerr << "Failed to receive player pos" << std::endl;
        return;
    }

    if (sendAndCheckAnswer(&player->tcpSocket, "caq.has", "caq.yes"))
    {
        player->tcpSocket.write("caq.req");

        ActionPacket ap = {};
        player->tcpSocket.read(&ap, sizeof(ActionPacket));

        if (!sendAndCheckAnswer(&player->tcpSocket, "caq.recv", "caq.end"))
        {
            std::cerr << "Player Sync Failed to complete packet request (nok)" << std::endl;
            return;
        }

        if (ap.type == EActionPacketType::CHAT_SEND)
        {
            std::string msg = ap.data;

            if (msg.length() > 0 && msg[0] == '/')
            {
                if (msg.substr(1, msg.find(' ')-1) == "kick")
                {
                    std::string playerName = msg.substr(msg.find(' ')+1);

                    for (size_t i = 0; i < players.size(); ++i)
                    {
                        if (players[i]->username == playerName)
                        {
                            players[i]->disconnect = true;
                            break;
                        }
                    }

                }
            }
            else if (msg.length() > 0)
            {
                std::string str = player->username + ": " + msg;
                ap.type = EActionPacketType::CHAT_RECEIVED;
                strcpy(ap.data, str.c_str());

                for (size_t i = 0; i < players.size(); ++i)
                {
                    if (!players[i]->disconnect)
                        players[i]->actionQueue.push(ap);
                }
            }
        }
    }
    else
    {
        if (!sendAndCheckAnswer(&player->tcpSocket, "caq.ok", "caq.end"))
        {
            std::cerr << "Player Sync Failed to complete packet request (nok)" << std::endl;
            return;
        }
    }

    if (!sendAndCheckAnswer(&player->tcpSocket, "sync.end", "sync.ok"))
    {
        std::cerr << "Player Sync Failed to complete (nok)" << std::endl;
        return;
    }
}