#include "Chat.h"

#include "Global.h"

void Chat::Init()
{
    m_chatFont.loadFromFile("Poppins-Regular.ttf");

    m_inputText.setFont(m_chatFont);
    m_inputText.setCharacterSize(20);
    m_inputText.setFillColor(sf::Color::Black);
}

std::string keyToString(const sf::Keyboard::Key& key)
{
    switch (key)
    {
        case sf::Keyboard::A:
            return "a";
        case sf::Keyboard::B:
            return "b";
        case sf::Keyboard::C:
            return "c";
        case sf::Keyboard::D:
            return "d";
        case sf::Keyboard::E:
            return "e";
        case sf::Keyboard::F:
            return "f";
        case sf::Keyboard::G:
            return "g";
        case sf::Keyboard::H:
            return "h";
        case sf::Keyboard::I:
            return "i";
        case sf::Keyboard::J:
            return "j";
        case sf::Keyboard::K:
            return "k";
        case sf::Keyboard::L:
            return "l";
        case sf::Keyboard::M:
            return "m";
        case sf::Keyboard::N:
            return "n";
        case sf::Keyboard::O:
            return "o";
        case sf::Keyboard::P:
            return "p";
        case sf::Keyboard::Q:
            return "q";
        case sf::Keyboard::R:
            return "r";
        case sf::Keyboard::S:
            return "s";
        case sf::Keyboard::T:
            return "t";
        case sf::Keyboard::U:
            return "u";
        case sf::Keyboard::V:
            return "v";
        case sf::Keyboard::W:
            return "w";
        case sf::Keyboard::X:
            return "x";
        case sf::Keyboard::Y:
            return "y";
        case sf::Keyboard::Z:
            return "z";
        case sf::Keyboard::Space:
            return " ";
        case sf::Keyboard::Num0:
            return "0";
        case sf::Keyboard::Num1:
            return "1";
        case sf::Keyboard::Num2:
            return "2";
        case sf::Keyboard::Num3:
            return "3";
        case sf::Keyboard::Num4:
            return "4";
        case sf::Keyboard::Num5:
            return "5";
        case sf::Keyboard::Num6:
            return "6";
        case sf::Keyboard::Num7:
            return "7";
        case sf::Keyboard::Num8:
            return "8";
        case sf::Keyboard::Num9:
            return "9";
        default:
            return "";
    }
}

void Chat::ProcessEvent(sf::Event &event)
{
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Enter)
        {
            if (!m_isInputting) m_isInputting = true;
            else
            {
                if (m_input.length() > 0)
                {
                    ActionPacket ap = {};
                    ap.type = EActionPacketType::CHAT_SEND;
                    std::string str = m_input;
#ifdef _WIN32
                    strcpy_s(ap.data, str.c_str());
#else
                    strcpy(ap.data, str.c_str());
#endif
                    Global::GetActionQueue().push(ap);
                }

                m_input = "";
                m_isInputting = false;
            }
        }

        if (event.key.code == sf::Keyboard::LShift || event.key.code == sf::Keyboard::RShift)
            m_shiftDown = true;

        if (m_isInputting)
        {
            if (event.key.code == sf::Keyboard::Escape)
            {
                m_isInputting = false;
                m_input = "";
            }

            if (event.key.code == sf::Keyboard::BackSpace && m_input.length() > 0)
                m_input.pop_back();

            std::string str = keyToString(event.key.code);
            if (m_shiftDown)
                std::transform(str.begin(), str.end(), str.begin(), ::toupper);
            if (str == "7" && m_shiftDown) str = "/";
            m_input += str;
        }
    }
    else if (event.type == sf::Event::KeyReleased)
    {
        if (event.key.code == sf::Keyboard::LShift || event.key.code == sf::Keyboard::RShift)
            m_shiftDown = false;
    }
}

void Chat::Render(sf::RenderWindow &window)
{
    float chatWidth = 580.f;
    float chatHeight = 350.f;
    float inputHeight = 40.f;

    sf::Vector2u windowSize = window.getSize();

    sf::RectangleShape chatBox(sf::Vector2f(chatWidth,chatHeight-inputHeight));
    sf::RectangleShape chatInput(sf::Vector2f(chatWidth,inputHeight));

    chatBox.setFillColor(sf::Color(255,255,255,125));
    chatBox.setPosition((float)windowSize.x-chatWidth-10.f, (windowSize.y/2)-(chatHeight/2));
    chatInput.setFillColor(sf::Color(255,255,255,125));
    if (m_isInputting)
    {
        chatInput.setOutlineThickness(5);
        chatInput.setOutlineColor(sf::Color::Blue);
    }
    else
    {
        chatInput.setOutlineThickness(0);
    }
    chatInput.setPosition((float)windowSize.x-chatWidth-10.f,((windowSize.y/2)-(chatHeight/2))+(chatHeight-inputHeight*2));

    window.draw(chatBox);
    window.draw(chatInput);

    uint8_t index;
    size_t i;

    if (m_chatMessages.size() > 5)
    {
        i = m_chatMessages.size()-6;
        index = 6;
    }
    else
    {
        i = 0;
        index = m_chatMessages.size();
    }

    while (i < m_chatMessages.size())
    {
        m_chatMessages[i].setPosition((float)windowSize.x-chatWidth, (((windowSize.y/2)-(chatHeight/2))+(chatHeight-inputHeight*2))-(40*index));
        window.draw(m_chatMessages[i]);
        --index;
        ++i;
    }

    m_inputText.setString(m_input);
    m_inputText.setPosition((float)windowSize.x-chatWidth, ((windowSize.y/2)-(chatHeight/2))+(chatHeight-inputHeight*2)+7);
    window.draw(m_inputText);
}

void Chat::AddMsg(const std::string &msg)
{
    sf::Text chatMsg;
    chatMsg.setFont(m_chatFont);
    chatMsg.setFillColor(sf::Color::Black);
    chatMsg.setCharacterSize(25);
    chatMsg.setString(msg);
    m_chatMessages.push_back(chatMsg);
}
