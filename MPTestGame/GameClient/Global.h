#pragma once

#include <queue>

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

namespace Global
{
    std::queue<ActionPacket>& GetActionQueue();
    const std::string& GetUsername();
}