#include <SFML/Graphics.hpp>
#include <sockpp/tcp_connector.h>
#include <thread>
#include <cmath>

#include "Global.h"
#include "Player.h"
#include "Timer.h"
#include "Chat.h"

int16_t port = 5110;
char hostname[] = "localhost";

std::map<std::string, Player*> netPlayers;
Player player;

std::string username;

std::queue<ActionPacket> actionQueue;
std::queue<ActionPacket>& Global::GetActionQueue() { return actionQueue; }

const std::string& Global::GetUsername() { return username; }

Chat chat;

void netTask(sockpp::tcp_connector conn);

void debugPrintf(const std::string& str)
{
    //printf("%s", str.c_str());
}

int main()
{
    sf::RenderWindow window = { sf::VideoMode(1920, 1080), "Game Window" };

    std::cin >> username;

    std::cout << "Connecting to server..." << std::endl;

    sockpp::socket_initializer sockInit;

    sockpp::tcp_connector conn;
    if (!conn.connect(sockpp::inet_address(hostname, port)))
    {
        std::cerr << "Failed to connect to server " << hostname << ":" << port << " (" << conn.last_error_str() << ")" << std::endl;
        return -1;
    }

    std::cout << "Logging in..." << std::endl;

    conn.read_timeout(std::chrono::duration<float, std::milli>(5000.f));

    conn.write("logon");
    char buffer[512];
    ssize_t n;
    if ((n = conn.read(buffer, sizeof(buffer))) > 0)
    {
        conn.write(username);
    }
    else
    {
        std::cerr << "Timed out" << std::endl;
        return -1;
    }

    std::cout << "Connected to server." << std::endl;

    chat.Init();

    Timer timer;
    timer.Start();

    player.Start();

    sf::Text fpsTxt;
    fpsTxt.setFillColor(sf::Color::White);
    fpsTxt.setCharacterSize(25);
    fpsTxt.setPosition(0, 0);
    sf::Font font;
    font.loadFromFile("Poppins-Regular.ttf");
    fpsTxt.setFont(font);

    std::thread thr(netTask, std::move(conn));
    thr.detach();

    while (window.isOpen())
    {
        float ms = timer.ElapsedMilliseconds();
        float seconds = timer.ElapsedSeconds();
        uint32_t fps = static_cast<uint32_t>(roundf(1000.f / ms));
        timer.Restart();

        fpsTxt.setString("FPS: " + sf::String(std::to_string(fps)));

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            chat.ProcessEvent(event);
        }

        if (window.hasFocus())
            player.Update(seconds);

        window.clear(sf::Color::Black);

        auto itr = netPlayers.begin();
        while (itr != netPlayers.end())
        {
            itr->second->Render(&window);
            ++itr;
        }

        player.Render(&window);

        chat.Render(window);

        window.draw(fpsTxt);
        window.display();
    }

    return 0;
}

struct NetPlayerUpdate
{
    char username[64];
    float xPos, yPos;
    bool isLast;
};



void doServerSync(sockpp::tcp_connector* conn);
void onlinePlayersPosSync(sockpp::tcp_connector* conn);
void aqSync(sockpp::tcp_connector* conn);

void netTask(sockpp::tcp_connector conn)
{
    char buffer[256] = "\0";
    ssize_t n;

    while ((n = conn.read(buffer, sizeof(buffer))) > 0)
    {
        if (std::string(buffer) == "sync.req")
        {
            debugPrintf("Received sync rec sending ack\n");
            conn.write("sync.ack");
            doServerSync(&conn);
        }
    }
}

void clearBuffer(char* buffer, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        buffer[i] = '\0';
    }
}

void doServerSync(sockpp::tcp_connector* conn)
{
    char buffer[128] = "\0";

    ssize_t n = conn->read(buffer, sizeof(buffer));
    if (n <= 0)
    {
        std::cerr << "Server sync failed" << std::endl;
        return;
    }

    if (std::string(buffer) == "aq.has")
    {
        debugPrintf("Received aq has\n");
        conn->write("aq.ok");

        aqSync(conn);
    }
    else if (std::string(buffer) == "aq.none")
    {
        debugPrintf("Received aq none writing ok\n");
        conn->write("aq.ok");
    }

    clearBuffer(buffer, sizeof(buffer));
    n = conn->read(buffer, sizeof(buffer));
    if (n <= 0)
    {
        std::cerr << "Server sync failed" << std::endl;
        return;
    }

    if (std::string(buffer) == "ppos.begin")
    {
        debugPrintf("received ppos begin sending ok\n");
        conn->write("ppos.ok");

        onlinePlayersPosSync(conn);
        clearBuffer(buffer, sizeof(buffer));
        n = conn->read(buffer, sizeof(buffer));
        if (n <= 0)
        {
            std::cerr << "Server sync failed" << std::endl;
            return;
        }

        if (std::string(buffer) == "ppos.end")
        {
            debugPrintf("received ppos end writing end\n");
            conn->write("ppos.end");
        }
    }
    else if (std::string(buffer) == "ppos.none")
    {
        debugPrintf("received ppos none writing ok\n");
        conn->write("ppos.ok");
    }

    clearBuffer(buffer, sizeof(buffer));
    n = conn->read(buffer, sizeof(buffer));
    if (n <= 0)
    {
        std::cerr << "Server sync failed" << std::endl;
        return;
    }

    if (std::string(buffer) == "pos.req")
    {
        NetPlayerUpdate upd = {};
        upd.xPos = player.s_xPos;
        upd.yPos = player.s_yPos;
        conn->write(&upd, sizeof(upd));

        clearBuffer(buffer, sizeof(buffer));
        n = conn->read(buffer, sizeof(buffer));
        if (n <= 0)
        {
            std::cerr << "Server sync failed" << std::endl;
            return;
        }

        if (std::string(buffer) == "pos.recv")
        {
            conn->write("pos.end");
        }
    }

    clearBuffer(buffer, sizeof(buffer));
    n = conn->read(buffer, sizeof(buffer));
    if (n <= 0)
    {
        std::cerr << "Server sync failed" << std::endl;
        return;
    }

    if (std::string(buffer) == "caq.has")
    {
        if (!actionQueue.empty())
        {
            conn->write("caq.yes");

            clearBuffer(buffer, sizeof(buffer));
            n = conn->read(buffer, sizeof(buffer));
            if (n <= 0)
            {
                std::cerr << "Server sync failed" << std::endl;
                return;
            }

            if (std::string(buffer) == "caq.req")
            {
                conn->write(&actionQueue.front(), sizeof(ActionPacket));
                actionQueue.pop();

                clearBuffer(buffer, sizeof(buffer));
                n = conn->read(buffer, sizeof(buffer));
                if (n <= 0)
                {
                    std::cerr << "Server sync failed" << std::endl;
                    return;
                }

                if (std::string(buffer) == "caq.recv")
                {
                    conn->write("caq.end");
                }
            }
        }
        else
        {
            conn->write("caq.no");

            clearBuffer(buffer, sizeof(buffer));
            n = conn->read(buffer, sizeof(buffer));
            if (n <= 0)
            {
                std::cerr << "Server sync failed" << std::endl;
                return;
            }

            if (std::string(buffer) == "caq.ok")
            {
                conn->write("caq.end");
            }
        }
    }

    clearBuffer(buffer, sizeof(buffer));
    n = conn->read(buffer, sizeof(buffer));
    if (n <= 0)
    {
        std::cerr << "Server sync failed" << std::endl;
        return;
    }

    if (std::string(buffer) == "sync.end")
    {
        debugPrintf("received sync end writing ok\n");
        conn->write("sync.ok");
    }
}

void aqSync(sockpp::tcp_connector* conn)
{
    ActionPacket packet = {};

    ssize_t n = conn->read(&packet, sizeof(ActionPacket));
    if (n <= 0)
    {
        std::cerr << "Failed AQ Sync" << std::endl;
        return;
    }

    conn->write("aq.recv");

    if (packet.type == EActionPacketType::PLAYER_JOINED)
    {
        std::cout << "PLayer joined: " << packet.data << std::endl;
        std::string msg = std::string(packet.data) + " has joined the game";
        chat.AddMsg(msg);
        auto p = new Player();
        p->Start();
        netPlayers.emplace(std::pair<std::string, Player*>(packet.data, p));
    }
    else if (packet.type == EActionPacketType::PLAYER_LEFT)
    {
        std::cout << "Player left: " << packet.data << std::endl;
        std::string msg = std::string(packet.data) + " has left the game";
        chat.AddMsg(msg);

        auto found = netPlayers.find(packet.data);
        if (found != netPlayers.end())
        {
            netPlayers.erase(found);
        }
    }
    else if (packet.type == EActionPacketType::CHAT_RECEIVED)
    {
        chat.AddMsg(packet.data);
    }
}

void onlinePlayersPosSync(sockpp::tcp_connector *conn)
{
    ssize_t n;
    NetPlayerUpdate upd = {};

    while ((n = conn->read(&upd, sizeof(NetPlayerUpdate))) > 0)
    {
        auto found = netPlayers.find(upd.username);
        if (found != netPlayers.end())
        {
            found->second->SetPosition(upd.xPos, upd.yPos);
        }

        debugPrintf("received a position sending rev\n");

        conn->write("ppos.recv");

        if (upd.isLast) break;
    }

    debugPrintf("done with pos\n");
}
