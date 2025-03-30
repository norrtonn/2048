#include <SFML/Graphics.hpp>
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

// Константы игры
const int GRID_SIZE = 4;
const int TILE_SIZE = 100;
const int TILE_MARGIN = 10;
const int WINDOW_WIDTH = GRID_SIZE * (TILE_SIZE + TILE_MARGIN) + TILE_MARGIN;
const int WINDOW_HEIGHT = GRID_SIZE * (TILE_SIZE + TILE_MARGIN) + TILE_MARGIN;
const float ANIMATION_DURATION = 0.1f; // Продолжительность анимации в секундах

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
};

std::vector<Animation> animations;

// Прототипы функций
void addRandomTile();
void initGame();

sf::Color getTileColor(int value) {
    switch (value) {
    case 0: return sf::Color(205, 193, 180); // Пустая плитка
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
    default: return sf::Color(60, 58, 50); // Для плиток > 2048
    }
}

void initGame() {
    // Очистка поля
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = 0;
        }
    }

    // Сброс счёта и статусов
    score = 0;
    gameOver = false;
    win = false;

    // Добавляем начальные плитки
    addRandomTile();
    addRandomTile();
}

void addRandomTile() {
    // Список пустых ячеек
    int emptyCells[GRID_SIZE * GRID_SIZE][2];
    int count = 0;

    // Находим все пустые ячейки
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 0) {
                emptyCells[count][0] = i;
                emptyCells[count][1] = j;
                count++;
            }
        }
    }

    // Если есть пустые ячейки
    if (count > 0) {
        // Выбор случайной пустой ячейки
        int index = rand() % count;
        int row = emptyCells[index][0];
        int col = emptyCells[index][1];

        // С вероятностью 90% появляется 2, иначе 4
        grid[row][col] = (rand() % 10 < 9) ? 2 : 4;
    }
}

bool canMove() {
    // Проверка, есть ли пустые ячейки
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 0) {
                return true;
            }
        }
    }

    // Проверка, есть ли соседние одинаковые плитки
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE - 1; j++) {
            if (grid[i][j] == grid[i][j + 1]) {
                return true;
            }
        }
    }

    for (int j = 0; j < GRID_SIZE; j++) {
        for (int i = 0; i < GRID_SIZE - 1; i++) {
            if (grid[i][j] == grid[i + 1][j]) {
                return true;
            }
        }
    }

    return false;
}

void updateGameState() {
    // Проверка на выигрыш (достижение плитки 2048)
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 2048) {
                win = true;
                return;
            }
        }
    }

    // Проверка на проигрыш (Невозможно сделать ход)
    if (!canMove()) {
        gameOver = true;
    }
}

bool moveDown() {
    bool moved = false;

    for (int j = 0; j < GRID_SIZE; j++) {
        // Сдвиг вниз
        for (int i = GRID_SIZE - 2; i >= 0; i--) {
            if (grid[i][j] != 0) {
                int row = i;
                while (row < GRID_SIZE - 1 && grid[row + 1][j] == 0) {
                    // Создаем анимацию
                    animations.push_back({ row, j, row + 1, j, grid[row][j], 0.0f });
                    grid[row + 1][j] = grid[row][j];
                    grid[row][j] = 0;
                    row++;
                    moved = true;
                }
            }
        }

        // Объединение
        for (int i = GRID_SIZE - 1; i > 0; i--) {
            if (grid[i][j] == grid[i - 1][j] && grid[i][j] != 0) {
                // Создаем анимацию
                animations.push_back({ i - 1, j, i, j, grid[i][j] * 2, 0.0f });

                grid[i][j] *= 2;
                score += grid[i][j];
                grid[i - 1][j] = 0;
                moved = true;
            }
        }

        // Повторный сдвиг
        for (int i = GRID_SIZE - 2; i >= 0; i--) {
            if (grid[i][j] != 0) {
                int row = i;
                while (row < GRID_SIZE - 1 && grid[row + 1][j] == 0) {
                    // Создаем анимацию
                    animations.push_back({ row, j, row + 1, j, grid[row][j], 0.0f });

                    grid[row + 1][j] = grid[row][j];
                    grid[row][j] = 0;
                    row++;
                    moved = true;
                }
            }
        }
    }
    return moved;
}

bool moveUp() {
    bool moved = false;

    for (int j = 0; j < GRID_SIZE; j++) {
        // Сдвиг вверх
        for (int i = 1; i < GRID_SIZE; i++) {
            if (grid[i][j] != 0) {
                int row = i;
                while (row > 0 && grid[row - 1][j] == 0) {
                    // Создаем анимацию
                    animations.push_back({ row, j, row - 1, j, grid[row][j], 0.0f });

                    grid[row - 1][j] = grid[row][j];
                    grid[row][j] = 0;
                    row--;
                    moved = true;
                }
            }
        }

        // Объединение
        for (int i = 0; i < GRID_SIZE - 1; i++) {
            if (grid[i][j] == grid[i + 1][j] && grid[i][j] != 0) {
                // Создаем анимацию
                animations.push_back({ i + 1, j, i, j, grid[i][j] * 2, 0.0f });
                grid[i][j] *= 2;
                score += grid[i][j];
                grid[i + 1][j] = 0;
                moved = true;
            }
        }

        // Повторный сдвиг
        for (int i = 1; i < GRID_SIZE; i++) {
            if (grid[i][j] != 0) {
                int row = i;
                while (row > 0 && grid[row - 1][j] == 0) {
                    // Создаем анимацию
                    animations.push_back({ row, j, row - 1, j, grid[row][j], 0.0f });
                    grid[row - 1][j] = grid[row][j];
                    grid[row][j] = 0;
                    row--;
                    moved = true;
                }
            }
        }
    }
    return moved;
}

bool moveRight() {
    bool moved = false;

    for (int i = 0; i < GRID_SIZE; i++) {
        // Сдвиг вправо
        for (int j = GRID_SIZE - 2; j >= 0; j--) {
            if (grid[i][j] != 0) {
                int col = j;
                while (col < GRID_SIZE - 1 && grid[i][col + 1] == 0) {
                    // Создаем анимацию
                    animations.push_back({ i, col, i, col + 1, grid[i][col], 0.0f });
                    grid[i][col + 1] = grid[i][col];
                    grid[i][col] = 0;
                    col++;
                    moved = true;
                }
            }
        }

        // Объединение
        for (int j = GRID_SIZE - 1; j > 0; j--) {
            if (grid[i][j] == grid[i][j - 1] && grid[i][j] != 0) {
                // Создаем анимацию
                animations.push_back({ i, j - 1, i, j, grid[i][j] * 2, 0.0f });
                grid[i][j] *= 2;
                score += grid[i][j];
                grid[i][j - 1] = 0;
                moved = true;
            }
        }

        // Повторный сдвиг
        for (int j = GRID_SIZE - 2; j >= 0; j--) {
            if (grid[i][j] != 0) {
                int col = j;
                while (col < GRID_SIZE - 1 && grid[i][col + 1] == 0) {
                    // Создаем анимацию
                    animations.push_back({ i, col, i, col + 1, grid[i][col], 0.0f });
                    grid[i][col + 1] = grid[i][col];
                    grid[i][col] = 0;
                    col++;
                    moved = true;
                }
            }
        }
    }
    return moved;
}

bool moveLeft() {
    bool moved = false;

    for (int i = 0; i < GRID_SIZE; i++) {
        // Сдвиг влево
        for (int j = 1; j < GRID_SIZE; j++) {
            if (grid[i][j] != 0) {
                int col = j;
                while (col > 0 && grid[i][col - 1] == 0) {
                    // Создаем анимацию
                    animations.push_back({ i, col, i, col - 1, grid[i][col], 0.0f });
                    grid[i][col - 1] = grid[i][col];
                    grid[i][col] = 0;
                    col--;
                    moved = true;
                }
            }
        }

        // Объединение
        for (int j = 0; j < GRID_SIZE - 1; j++) {
            if (grid[i][j] == grid[i][j + 1] && grid[i][j] != 0) {
                // Создаем анимацию
                animations.push_back({ i, j + 1, i, j, grid[i][j] * 2, 0.0f });
                grid[i][j] *= 2;
                score += grid[i][j];
                grid[i][j + 1] = 0;
                moved = true;
            }
        }

        // Повторный сдвиг
        for (int j = 1; j < GRID_SIZE; j++) {
            if (grid[i][j] != 0) {
                int col = j;
                while (col > 0 && grid[i][col - 1] == 0) {
                    // Создаем анимацию
                    animations.push_back({ i, col, i, col - 1, grid[i][col], 0.0f });
                    grid[i][col - 1] = grid[i][col];
                    grid[i][col] = 0;
                    col--;
                    moved = true;
                }
            }
        }
    }
    return moved;
}

void drawGrid(sf::RenderWindow& window, sf::Font& font) {
    // Рисовка фона игрового поля
    sf::RectangleShape background(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    background.setFillColor(sf::Color(187, 173, 160));
    window.draw(background);

    // Рисовка плиток и их значения
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            // Рисуем только если нет анимации
            bool hasAnimation = false;
            for (const auto& anim : animations) {
                if (anim.rowStart == i && anim.colStart == j) {
                    hasAnimation = true;
                    break;
                }
                if (anim.rowEnd == i && anim.colEnd == j) {
                    hasAnimation = true;
                    break;
                }
            }

            if (!hasAnimation) {
                sf::RectangleShape tile(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                tile.setPosition(j * (TILE_SIZE + TILE_MARGIN) + TILE_MARGIN,
                    i * (TILE_SIZE + TILE_MARGIN) + TILE_MARGIN);
                tile.setFillColor(getTileColor(grid[i][j]));
                window.draw(tile);

                // Отображение значения, если плитка не пустая
                if (grid[i][j] != 0) {
                    sf::Text text;
                    text.setFont(font);
                    text.setString(std::to_string(grid[i][j]));

                    // Настройка размера текста в зависимости от числа
                    if (grid[i][j] < 100)
                        text.setCharacterSize(50);
                    else if (grid[i][j] < 1000)
                        text.setCharacterSize(40);
                    else
                        text.setCharacterSize(30);

                    // Настройка цвета текста
                    text.setFillColor(grid[i][j] <= 4 ? sf::Color(119, 110, 101) : sf::Color(249, 246, 242));

                    // Центрирование текста
                    sf::FloatRect textRect = text.getLocalBounds();
                    text.setOrigin(textRect.left + textRect.width / 2.0f,
                        textRect.top + textRect.height / 2.0f);
                    text.setPosition(j * (TILE_SIZE + TILE_MARGIN) + TILE_MARGIN +
                        TILE_SIZE / 2.0f,
                        i * (TILE_SIZE + TILE_MARGIN) + TILE_MARGIN +
                        TILE_SIZE / 2.0f);

                    window.draw(text);
                }
            }
        }
    }

    // Рисуем анимации
    for (const auto& anim : animations) {
        float progress = anim.time / ANIMATION_DURATION;
        if (progress > 1.0f) progress = 1.0f;

        float x = (anim.colStart + (anim.colEnd - anim.colStart) * progress) * (TILE_SIZE + TILE_MARGIN) + TILE_MARGIN;
        float y = (anim.rowStart + (anim.rowEnd - anim.rowStart) * progress) * (TILE_SIZE + TILE_MARGIN) + TILE_MARGIN;

        sf::RectangleShape tile(sf::Vector2f(TILE_SIZE, TILE_SIZE));
        tile.setPosition(x, y);
        tile.setFillColor(getTileColor(anim.value));
        window.draw(tile);

        // Отображение значения, если плитка не пустая
        if (anim.value != 0) {
            sf::Text text;
            text.setFont(font);
            text.setString(std::to_string(anim.value));

            // Настройка размера текста в зависимости от числа
            if (anim.value < 100)
                text.setCharacterSize(50);
            else if (anim.value < 1000)
                text.setCharacterSize(40);
            else
                text.setCharacterSize(30);

            // Настройка цвета текста
            text.setFillColor(anim.value <= 4 ? sf::Color(119, 110, 101) : sf::Color(249, 246, 242));

            // Центрирование текста
            sf::FloatRect textRect = text.getLocalBounds();
            text.setOrigin(textRect.left + textRect.width / 2.0f,
                textRect.top + textRect.height / 2.0f);
            text.setPosition(x + TILE_SIZE / 2.0f, y + TILE_SIZE / 2.0f);

            window.draw(text);
        }
    }

    // Отображение счета счет
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setString("Score: " + std::to_string(score));
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 10);
    window.draw(scoreText);

    // Отображение сообщения о конце игры или победе
    sf::Text statusText;
    statusText.setFont(font);
    statusText.setCharacterSize(40);
    statusText.setFillColor(sf::Color::White);
    statusText.setPosition(WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 50);

    if (gameOver) {
        statusText.setString("Game over!");
        window.draw(statusText);
    }

    if (win) {
        statusText.setString("You win!");
        window.draw(statusText);
    }
}

int main() {
    system("chcp 1251");

    // Инициализация генератор случайных чисел
    srand(time(0));

    // Создание окна
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2048");
    window.setFramerateLimit(60);

    // Загрузка шрифта
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        return 1;
    }

    // Инициализация игры
    initGame();

    // Основной игровой цикл
    sf::Clock clock;
    while (window.isOpen()) {
        sf::Time deltaTime = clock.restart();
        float dt = deltaTime.asSeconds();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Обработка нажатия клавиш
            if (event.type == sf::Event::KeyPressed) {
                bool moved = false;

                // Управление стрелками или WASD
                if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::A) {
                    moved = moveLeft();
                }
                else if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::D) {
                    moved = moveRight();
                }
                else if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
                    moved = moveUp();
                }
                else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
                    moved = moveDown();
                }

                if (moved) {
                    addRandomTile();
                    updateGameState();
                }

                // Рестарт игры по нажатию R
                if (event.key.code == sf::Keyboard::R) {
                    initGame();
                }

                // Выход из игры по Escape
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }
        }

        // Обновление анимаций
        for (auto& anim : animations) {
            anim.time += dt;
        }

        // Удаление завершенных анимаций
        animations.erase(std::remove_if(animations.begin(), animations.end(),
            [](const Animation& anim) { return anim.time >= ANIMATION_DURATION; }), animations.end());

        // Отрисовка
        window.clear();
        drawGrid(window, font);
        window.display();
    }

    return 0;
}
