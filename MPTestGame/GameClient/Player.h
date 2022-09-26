#pragma once

#include <SFML/Graphics.hpp>

class Player
{
public:
    Player();

    void Start();
    void Update(float deltaTime);
    void Render(sf::RenderWindow* window);

    void SetPosition(float x, float y)
    {
        s_xPos = x;
        s_yPos = y;

        m_circle.setPosition(s_xPos, s_yPos);
    }

    float s_xPos, s_yPos;

private:
    sf::CircleShape m_circle;

    std::string s_name;


};