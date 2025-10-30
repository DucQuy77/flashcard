#include <SFML/Graphics.hpp>
#include <vector>
#include <ctime>
#include <algorithm>

struct Card {
    sf::Sprite sprite;
    sf::RectangleShape back;
    bool revealed = false;
    bool matched = false;
    int id;
    bool flipping = false;
    float flipProgress = 0.f;
    bool showingFront = false;
    float originalScaleX;
    sf::Vector2f originalPos;
    sf::Vector2f backBasePos;
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Flashcard Match Game");
    window.setFramerateLimit(60);

    // Load ảnh
    sf::Texture textures[8];
    for (int i = 0; i < 8; i++)
        textures[i].loadFromFile("images/" + std::to_string(i) + ".jpg");

    // Tạo danh sách id (2 thẻ cho mỗi hình)
    std::vector<int> ids;
    for (int i = 0; i < 8; i++) {
        ids.push_back(i);
        ids.push_back(i);
    }

    // Xáo trộn ID (thứ tự hình)
    std::srand((unsigned)time(0));
    std::random_shuffle(ids.begin(), ids.end());

    // Tạo 16 thẻ với vị trí cố định
    std::vector<Card> cards;
    float cardWidth = 120, cardHeight = 120, spacing = 20;

    float totalWidth = 4 * cardWidth + 3 * spacing;
    float totalHeight = 4 * cardHeight + 3 * spacing;
    float startX = (800 - totalWidth) / 2;
    float startY = (600 - totalHeight) / 2;

    for (int i = 0; i < 16; i++) {
        Card c;
        c.id = ids[i];
        c.sprite.setTexture(textures[c.id]);

        // Scale ảnh cho vừa khung
        sf::Vector2u size = textures[c.id].getSize();
        float scale = std::min(cardWidth / size.x, cardHeight / size.y);
        c.sprite.setScale(scale, scale);

        // Tính cột, hàng và offset
        int col = i % 4;
        int row = i / 4;
        float x = startX + col * (cardWidth + spacing);
        float y = startY + row * (cardWidth + spacing);

        float offsetX = (cardWidth - size.x * scale) / 2;
        float offsetY = (cardHeight - size.y * scale) / 2;
        c.originalScaleX = scale;
        c.originalPos = sf::Vector2f(x + offsetX, y + offsetY);

        c.sprite.setPosition(c.originalPos);

        // Tạo mặt sau
        c.back.setSize(sf::Vector2f(cardWidth, cardHeight));
        c.back.setPosition(x, y);
        c.backBasePos = sf::Vector2f(x,y);
        c.back.setFillColor(sf::Color(180, 180, 180));
        c.back.setOutlineColor(sf::Color::Black);
        c.back.setOutlineThickness(2);

        cards.push_back(c);
    }

    sf::Font font;
    if (!font.loadFromFile("font/arial.ttf")) {
        return -1;
    }

    sf::Text winMessage;
    winMessage.setFont(font);
    winMessage.setString(L"CHÚC MỪNG! BẠN ĐÃ THẮNG!");
    winMessage.setCharacterSize(40);
    winMessage.setFillColor(sf::Color::Red);
    winMessage.setStyle(sf::Text::Bold);

    //Căn giữa thông báo trên màn hình
    sf::FloatRect textRect = winMessage.getLocalBounds();
    winMessage.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    winMessage.setPosition(sf::Vector2f(800/2.0f, 600/2.0f));

    int matchedPairs = 0;
    bool gameWon = false;

    int first = -1, second = -1;
    sf::Clock timer;
    bool waiting = false;

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e))
            if (e.type == sf::Event::Closed) window.close();

        if (!waiting && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            sf::Vector2f mouse(sf::Mouse::getPosition(window));
            for (int i = 0; i < cards.size(); i++) {
                if (cards[i].matched || cards[i].revealed) {
                    continue;
                }

                if (cards[i].back.getGlobalBounds().contains(mouse)) {
                    cards[i].flipping = true;
                    break;
                }
            }
        }

        // Cập nhật hiệu ứng lật bài
        for (int i = 0; i < cards.size(); i++)
        {
            Card& c = cards[i];
            if (c.flipping) {
                c.flipProgress += 0.03f;

                if (c.flipProgress >= 0.6f && !c.showingFront) {
                    c.showingFront = true;
                }

                if (c.flipProgress >= 1.f) {
                    c.flipProgress = 0.f;
                    c.flipping = false;
                    c.revealed = true;
                    c.showingFront = true;

                    if (first == -1) {
                        first = i;
                    }
                    else if (second == -1 && i != first) {
                        second = i;
                        waiting = true;
                        timer.restart();
                    }
                }
            }
        }

        if (waiting && timer.getElapsedTime().asSeconds() > 1) {
            if (cards[first].id == cards[second].id) {
                cards[first].matched = cards[second].matched = true;
                matchedPairs++;
            }
            else
                cards[first].revealed = cards[second].revealed = false;
                cards[first].showingFront = cards[second].showingFront = false;

            first = second = -1;
            waiting = false;
        }
        if (!gameWon & matchedPairs == 8) {
            gameWon = true;
        }

        //Vẽ
        window.clear(sf::Color::White);
        for (auto& c : cards) {
            if (c.matched) continue;

            float scaleX = 1.f;
            if (c.flipping)
                scaleX = 1.f - std::abs(c.flipProgress - 0.5f) * 2.f;
            if (c.showingFront) {
                c.sprite.setScale(scaleX * c.originalScaleX, c.originalScaleX);
                
                float cardWidth = 120;
                float cardBackX = c.back.getPosition().x;
                float currentTotalWidth = cardWidth * scaleX;
                float diffX = (cardWidth - currentTotalWidth) / 2.f;

                float newPos = cardBackX + diffX + (c.originalPos.x - cardBackX);
            
                c.sprite.setPosition(newPos, c.originalPos.y);

                window.draw(c.sprite);
            }
            else {
                //Chỉ đặt tỷ lệ cho mặt sau
                c.back.setScale(scaleX, 1.f);

                //Tính toán lại vị trí của c.back để nó co lại vào giữa
                float cardWidth = 120;
                float currentTotalWidth = cardWidth * scaleX;
                float diffX = (cardWidth - currentTotalWidth) / 2.f;

                c.back.setPosition(c.backBasePos.x + diffX, c.backBasePos.y);

                window.draw(c.back);
            }
        }
        if (gameWon) {
            window.draw(winMessage);
        }
        window.display();
    }
}
