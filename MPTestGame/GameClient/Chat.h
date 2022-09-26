#pragma once

#include <SFML/Graphics.hpp>

class Chat
{
public:
    void Init();
    void ProcessEvent(sf::Event& event);
    void Render(sf::RenderWindow& window);

    void AddMsg(const std::string& msg);

    bool HasMessageToSend = false;
    std::string MessageToSend;

private:
    sf::Font m_chatFont;
    int m_chatLineFocus;

    std::vector<sf::Text> m_chatMessages;

    sf::Text m_inputText;
    bool m_isInputting = false;
    std::string m_input;
    bool m_shiftDown;

};
