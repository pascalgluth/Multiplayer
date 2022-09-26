#include "Player.h"

Player::Player()
{
    s_xPos = 100.f;
    s_yPos = 100.f;
}

void Player::Start()
{
    m_circle.setRadius(100.f);
    m_circle.setFillColor(sf::Color::Cyan);
}

void Player::Update(float deltaTime)
{
    float speed = 1000000.f * deltaTime;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        s_xPos -= speed * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        s_xPos += speed * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        s_yPos -= speed * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        s_yPos += speed * deltaTime;

    m_circle.setPosition(s_xPos, s_yPos);
}

void Player::Render(sf::RenderWindow* window)
{
    window->draw(m_circle);
}
