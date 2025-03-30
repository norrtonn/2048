#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>

// Константы игры
const int GRID_SIZE = 4;
const int TILE_SIZE = 100;
const int TILE_MARGIN = 10;
const int WINDOW_WIDTH = GRID_SIZE * (TILE_SIZE + TILE_MARGIN) + TILE_MARGIN;
const int WINDOW_HEIGHT = GRID_SIZE * (TILE_SIZE + TILE_MARGIN) + TILE_MARGIN + 50;
const float ANIMATION_DURATION = 0.1f;
const int BLUR_SIZE = 5;

// Игровые переменные
int grid[GRID_SIZE][GRID_SIZE] = { 0 };
int score = 0;
bool gameOver = false;
bool win = false;

// Структура для анимации плитки
struct Animation {
    int rowStart, colStart, rowEnd, colEnd;
    int value;
    float time;

    Animation(int rs, int cs, int re, int ce, int v, float t) :
        rowStart(rs), colStart(cs), rowEnd(re), colEnd(ce), value(v), time(t) {
    }
};

std::vector<Animation> animations;
std::vector<std::pair<int, int>> emptyCells;

// Прототипы функций
void addRandomTile();
void initGame();
void updateEmptyCells();
bool moveTiles(int direction);
sf::Color getTileColor(int value);
sf::Text getBlurredText(const sf::Text& text, sf::Font& font, int blurSize);
void drawGrid(sf::RenderWindow& window, sf::Font& font);

sf::Color getTileColor(int value) {
    switch (value) {
    case 0: return sf::Color(205, 193, 180);
    case 2: return sf::Color(238, 228, 218);
    case 4: return sf::Color(237, 224, 200);
    case 8: return sf::Color(242, 177, 121);
    case 16: return sf::Color(245, 149, 99);
    case 32: return sf::Color(246, 124, 95);
    case 64: return sf::Color(246, 94, 59);
    case 128: return sf::Color(237, 207, 114);
    case 256: return sf::Color(237, 204, 97);
    case 512: return sf::Color(237, 200, 80);
    case 1024: return sf::Color(237, 197, 63);
    case 2048: return sf::Color(237, 194, 46);
    default: return sf::Color(60, 58, 50);
    }
}

void initGame() {
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            grid[i][j] = 0;
        }
    }

    score = 0;
    gameOver = false;
    win = false;

    updateEmptyCells();
    addRandomTile();
    addRandomTile();
}

void updateEmptyCells() {
    emptyCells.clear();
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            if (grid[i][j] == 0) {
                emptyCells.emplace_back(i, j);
            }
        }
    }
}

void addRandomTile() {
    if (emptyCells.empty()) return;

    int index = rand() % emptyCells.size();
    int row = emptyCells[index].first;
    int col = emptyCells[index].second;

    grid[row][col] = (rand() % 10 < 9) ? 2 : 4;
    emptyCells.erase(emptyCells.begin() + index);
}

bool canMove() {
    if (!emptyCells.empty()) return true;

    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE - 1; ++j) {
            if (grid[i][j] == grid[i][j + 1]) return true;
        }
    }

    for (int j = 0; j < GRID_SIZE; ++j) {
        for (int i = 0; i < GRID_SIZE - 1; ++i) {
            if (grid[i][j] == grid[i + 1][j]) return true;
        }
    }

    return false;
}

void updateGameState() {
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            if (grid[i][j] == 2048) {
                win = true;
                return;
            }
        }
    }

    if (!canMove()) {
        gameOver = true;
    }
}

bool moveTiles(int direction) {
    bool moved = false;
    animations.clear();

    auto performMove = [&](int rowStart, int rowEnd, int rowIncrement, int colStart, int colEnd, int colIncrement) {
        for (int i = rowStart; i != rowEnd; i += rowIncrement) {
            for (int j = colStart; j != colEnd; j += colIncrement) {
                if (grid[i][j] != 0) {
                    int nextRow = i, nextCol = j;

                    // Find the farthest position to move
                    while (true) {
                        int newRow = nextRow, newCol = nextCol;
                        switch (direction) {
                        case 0: newRow--; break; // Up
                        case 1: newRow++; break; // Down
                        case 2: newCol--; break; // Left
                        case 3: newCol++; break; // Right
                        }

                        if (newRow < 0 || newRow >= GRID_SIZE || newCol < 0 || newCol >= GRID_SIZE) break;
                        if (grid[newRow][newCol] != 0) {
                            if (grid[newRow][newCol] == grid[i][j]) {
                                // Merge case
                                animations.emplace_back(i, j, newRow, newCol, grid[i][j] * 2, 0.0f);
                                score += grid[i][j] * 2;
                                grid[newRow][newCol] *= 2;
                                grid[i][j] = 0;
                                moved = true;
                            }
                            break;
                        }

                        nextRow = newRow;
                        nextCol = newCol;

                        animations.emplace_back(i, j, newRow, newCol, grid[i][j], 0.0f);
                        grid[newRow][newCol] = grid[i][j];
                        grid[i][j] = 0;
                        moved = true;
                    }
                }
            }
        }
        };

    switch (direction) {
    case 0: performMove(1, GRID_SIZE, 1, 0, GRID_SIZE, 1); break; // Up
    case 1: performMove(GRID_SIZE - 2, -1, -1, 0, GRID_SIZE, 1); break; // Down
    case 2: performMove(0, GRID_SIZE, 1, 1, GRID_SIZE, 1); break; // Left
    case 3: performMove(0, GRID_SIZE, 1, GRID_SIZE - 2, -1, -1); break; // Right
    }

    if (moved) {
        updateEmptyCells();
    }

    return moved;
}

sf::Text getBlurredText(const sf::Text& text, sf::Font& font, int blurSize) {
    sf::RenderTexture renderTexture;
    renderTexture.create(text.getLocalBounds().width + 2 * blurSize,
        text.getLocalBounds().height + 2 * blurSize);
    renderTexture.clear(sf::Color::Transparent);

    sf::Text tempText = text;
    tempText.setPosition(blurSize, blurSize);
    renderTexture.draw(tempText);
    renderTexture.display();

    sf::Sprite sprite(renderTexture.getTexture());

    std::string shaderCode =
        "uniform sampler2D texture;\n"
        "uniform float blur_size;\n"
        "void main() {\n"
        "    vec2 resolution = vec2(textureSize(texture, 0));\n"
        "    vec2 uv = gl_FragCoord.xy / resolution.xy;\n"
        "    vec4 color = vec4(0.0);\n"
        "    for (float i = -blur_size; i <= blur_size; i++) {\n"
        "        color += texture2D(texture, uv + vec2(i / resolution.x, 0.0));\n"
        "        color += texture2D(texture, uv + vec2(0.0, i / resolution.y));\n"
        "    }\n"
        "    color /= (2.0 * blur_size + 1.0) * 2.0 ;\n"
        "    gl_FragColor = color;\n"
        "}";

    sf::Shader shader;
    if (!shader.loadFromMemory(shaderCode, sf::Shader::Fragment)) {
        std::cerr << "Failed to load shader!" << std::endl;
    }

    shader.setUniform("texture", sf::Shader::CurrentTexture);
    shader.setUniform("blur_size", float(blurSize));

    sf::RenderStates renderStates;
    renderStates.shader = &shader;

    sf::RenderTexture blurredTexture;
    blurredTexture.create(text.getLocalBounds().width + 2 * blurSize,
        text.getLocalBounds().height + 2 * blurSize);
    blurredTexture.clear(sf::Color::Transparent);
    blurredTexture.draw(sprite, renderStates);
    blurredTexture.display();

    sf::Sprite blurredSprite(blurredTexture.getTexture());
    sf::Text blurredText;
    blurredText.setFont(font);
    blurredText.setString(text.getString());
    blurredText.setCharacterSize(text.getCharacterSize());
    blurredText.setFillColor(text.getFillColor());
    blurredText.setStyle(text.getStyle());
    blurredText.setPosition(text.getPosition());

    return text;
}

void drawGrid(sf::RenderWindow& window, sf::Font& font) {
    const float tileOffset = TILE_SIZE + TILE_MARGIN;

    sf::RectangleShape background(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT - 50));
    background.setFillColor(sf::Color(187, 173, 160));
    background.setPosition(0, 50);
    window.draw(background);

    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            bool hasAnimation = false;
            for (const auto& anim : animations) {
                if ((anim.rowStart == i && anim.colStart == j) || (anim.rowEnd == i && anim.colEnd == j)) {
                    hasAnimation = true;
                    break;
                }
            }

            if (!hasAnimation) {
                sf::RectangleShape tile(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                tile.setPosition(j * tileOffset + TILE_MARGIN, i * tileOffset + TILE_MARGIN + 50);
                tile.setFillColor(getTileColor(grid[i][j]));
                window.draw(tile);

                if (grid[i][j] != 0) {
                    sf::Text text;
                    text.setFont(font);
                    text.setString(std::to_string(grid[i][j]));

                    int charSize = (grid[i][j] < 100) ? 50 : ((grid[i][j] < 1000) ? 40 : 30);
                    text.setCharacterSize(charSize);
                    text.setFillColor(grid[i][j] <= 4 ? sf::Color(119, 110, 101) : sf::Color(249, 246, 242));

                    sf::FloatRect textRect = text.getLocalBounds();
                    text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
                    text.setPosition(j * tileOffset + TILE_MARGIN + TILE_SIZE / 2.0f,
                        i * tileOffset + TILE_MARGIN + TILE_SIZE / 2.0f + 50);
                    window.draw(text);
                }
            }
        }
    }

    for (const auto& anim : animations) {
        float progress = std::min(1.0f, anim.time / ANIMATION_DURATION);

        float x = (anim.colStart + (anim.colEnd - anim.colStart) * progress) * tileOffset + TILE_MARGIN;
        float y = (anim.rowStart + (anim.rowEnd - anim.rowStart) * progress) * tileOffset + TILE_MARGIN + 50;

        sf::RectangleShape tile(sf::Vector2f(TILE_SIZE, TILE_SIZE));
        tile.setPosition(x, y);
        tile.setFillColor(getTileColor(anim.value));
        window.draw(tile);

        if (anim.value != 0) {
            sf::Text text;
            text.setFont(font);
            text.setString(std::to_string(anim.value));

            int charSize = (anim.value < 100) ? 50 : ((anim.value < 1000) ? 40 : 30);
            text.setCharacterSize(charSize);
            text.setFillColor(anim.value <= 4 ? sf::Color(119, 110, 101) : sf::Color(249, 246, 242));

            sf::FloatRect textRect = text.getLocalBounds();
            text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            text.setPosition(x + TILE_SIZE / 2.0f, y + TILE_SIZE / 2.0f);
            window.draw(text);
        }
    }

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setString("Score: " + std::to_string(score));
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 10);
    window.draw(scoreText);

    if (gameOver) {
        sf::Text statusText;
        statusText.setFont(font);
        statusText.setString("Game over!");
        statusText.setCharacterSize(40);
        statusText.setFillColor(sf::Color::White);

        sf::FloatRect textRect = statusText.getLocalBounds();
        statusText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        statusText.setPosition(sf::Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f));

        sf::Text blurredText = getBlurredText(statusText, font, BLUR_SIZE);
        window.draw(blurredText);
    }

    if (win) {
        sf::Text statusText;
        statusText.setFont(font);
        statusText.setString("You win!");
        statusText.setCharacterSize(40);
        statusText.setFillColor(sf::Color::White);

        sf::FloatRect textRect = statusText.getLocalBounds();
        statusText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        statusText.setPosition(sf::Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f));
        window.draw(statusText);
    }
}

int main() {
    srand(time(0));

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2048");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        return 1;
    }

    initGame();

    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                int direction = -1;

                switch (event.key.code) {
                case sf::Keyboard::W:
                case sf::Keyboard::Up:      direction = 0; break;
                case sf::Keyboard::S:
                case sf::Keyboard::Down:    direction = 1; break;
                case sf::Keyboard::A:
                case sf::Keyboard::Left:    direction = 2; break;
                case sf::Keyboard::D:
                case sf::Keyboard::Right:   direction = 3; break;
                case sf::Keyboard::R:       initGame();     break;
                case sf::Keyboard::Escape:  window.close(); break;
                }

                if (direction != -1) {
                    if (moveTiles(direction)) {
                        addRandomTile();
                        updateGameState();
                    }
                }
            }
        }

        float deltaTime = clock.restart().asSeconds();
        for (auto& anim : animations) {
            anim.time += deltaTime;
        }

        animations.erase(std::remove_if(animations.begin(), animations.end(),
            [](const Animation& anim) {
                return anim.time >= ANIMATION_DURATION;
            }), animations.end());

        window.clear();
        drawGrid(window, font);
        window.display();
    }

    return 0;
}
