#include <algorithm>
#include <chrono>
#include <conio.h>     // _kbhit, _getch (Windows / MinGW)
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

struct Point {
    int x{};
    int y{};
};

enum class Direction {
    Up,
    Down,
    Left,
    Right
};

struct Settings {
    int width{40};
    int height{20};
    int baseDelayMs{120};  // 初始速度（帧间隔，越小越快）
    bool wallKills{true};  // true：撞墙死亡；false：穿墙模式
};

struct RareFood {
    Point pos{};
    bool active{false};
    int remainingTicks{0};
};

class SnakeGame {
public:
    SnakeGame() {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        loadHighScore();
    }

    void run() {
        for (;;) {
            showMainMenu();
        }
    }

private:
    Settings settings_;
    std::vector<Point> snake_;
    Direction dir_{Direction::Right};
    bool running_{false};
    bool paused_{false};
    Point food_{};
    RareFood rareFood_{};
    std::vector<Point> obstacles_;
    int score_{0};
    int highScore_{0};
    int level_{1};
    int ticks_{0};

private:
    // ---------- 公共工具 ----------
    void clearScreen() {
        // ANSI 清屏 + 光标复位（Git Bash / 大多数现代终端可用）
        std::cout << "\x1b[2J\x1b[H";
    }

    void sleepMs(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    bool pointsEqual(const Point &a, const Point &b) {
        return a.x == b.x && a.y == b.y;
    }

    bool containsPoint(const std::vector<Point> &v, const Point &p) {
        return std::any_of(v.begin(), v.end(), [&](const Point &q) { return pointsEqual(p, q); });
    }

    int randomInt(int lo, int hi) {
        static std::mt19937 rng(static_cast<unsigned int>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()));
        std::uniform_int_distribution<int> dist(lo, hi);
        return dist(rng);
    }

    // ---------- 存档 ----------
    void loadHighScore() {
        std::ifstream fin("highscore.txt");
        if (fin) {
            fin >> highScore_;
        }
    }

    void saveHighScore() {
        if (score_ > highScore_) {
            highScore_ = score_;
            std::ofstream fout("highscore.txt", std::ios::trunc);
            if (fout) {
                fout << highScore_;
            }
        }
    }

    // ---------- 菜单 ----------
    void waitForEnter() {
        std::cout << "\n按回车键继续...";
        while (true) {
            int ch = _getch();
            if (ch == '\r' || ch == '\n') {
                break;
            }
        }
    }

    void showMainMenu() {
        while (true) {
            clearScreen();
            std::cout << "================= 贪吃蛇 =================\n";
            std::cout << "最高分: " << highScore_ << "\n\n";
            std::cout << "1. 开始游戏\n";
            std::cout << "2. 设置\n";
            std::cout << "3. 退出\n\n";
            std::cout << "请输入选项编号并回车: ";

            std::string choice;
            if (!std::getline(std::cin, choice)) {
                return;
            }

            if (choice == "1") {
                startGame();
                return;  // 一局玩完回到 run()，再显示菜单
            } else if (choice == "2") {
                showSettingsMenu();
            } else if (choice == "3") {
                std::exit(0);
            } else {
                std::cout << "无效选项。\n";
                waitForEnter();
            }
        }
    }

    void showSettingsMenu() {
        while (true) {
            clearScreen();
            std::cout << "============= 设置 =============\n";
            std::cout << "1. 地图大小（当前: " << settings_.width << "x" << settings_.height << "）\n";
            std::cout << "2. 初始速度（当前: " << settings_.baseDelayMs << " ms/帧，数值越小越快）\n";
            std::cout << "3. 撞墙规则（当前: " << (settings_.wallKills ? "撞墙死亡" : "可穿墙") << "）\n";
            std::cout << "4. 返回主菜单\n\n";
            std::cout << "请选择: ";

            std::string choice;
            if (!std::getline(std::cin, choice)) {
                return;
            }

            if (choice == "1") {
                chooseMapSize();
            } else if (choice == "2") {
                chooseSpeed();
            } else if (choice == "3") {
                toggleWallRule();
            } else if (choice == "4") {
                return;
            } else {
                std::cout << "无效选项。\n";
                waitForEnter();
            }
        }
    }

    void chooseMapSize() {
        clearScreen();
        std::cout << "----- 选择地图大小 -----\n";
        std::cout << "1. 小 (30x16)\n";
        std::cout << "2. 中 (40x20)\n";
        std::cout << "3. 大 (60x24)\n\n";
        std::cout << "请输入选项编号并回车: ";
        std::string choice;
        if (!std::getline(std::cin, choice)) return;

        if (choice == "1") {
            settings_.width = 30;
            settings_.height = 16;
        } else if (choice == "2") {
            settings_.width = 40;
            settings_.height = 20;
        } else if (choice == "3") {
            settings_.width = 60;
            settings_.height = 24;
        } else {
            std::cout << "无效选项。\n";
            waitForEnter();
            return;
        }
    }

    void chooseSpeed() {
        clearScreen();
        std::cout << "----- 选择初始速度 -----\n";
        std::cout << "1. 慢 (180 ms/帧)\n";
        std::cout << "2. 中 (120 ms/帧)\n";
        std::cout << "3. 快 (80 ms/帧)\n\n";
        std::cout << "请输入选项编号并回车: ";
        std::string choice;
        if (!std::getline(std::cin, choice)) return;

        if (choice == "1") {
            settings_.baseDelayMs = 180;
        } else if (choice == "2") {
            settings_.baseDelayMs = 120;
        } else if (choice == "3") {
            settings_.baseDelayMs = 80;
        } else {
            std::cout << "无效选项。\n";
            waitForEnter();
        }
    }

    void toggleWallRule() {
        settings_.wallKills = !settings_.wallKills;
    }

    // ---------- 游戏初始化 ----------
    void resetGameState() {
        snake_.clear();
        int startX = settings_.width / 2;
        int startY = settings_.height / 2;
        snake_.push_back({startX, startY});
        snake_.push_back({startX - 1, startY});
        snake_.push_back({startX - 2, startY});
        dir_ = Direction::Right;
        score_ = 0;
        level_ = 1;
        ticks_ = 0;
        paused_ = false;
        obstacles_.clear();
        rareFood_ = RareFood{};

        generateObstacles();
        placeFood();
    }

    void generateObstacles() {
        // 随机放一些障碍物，但不要太多
        int area = settings_.width * settings_.height;
        int count = std::max(3, area / 200);  // 地图越大障碍越多
        obstacles_.clear();
        for (int i = 0; i < count; ++i) {
            Point p{};
            int tries = 0;
            do {
                p.x = randomInt(1, settings_.width - 2);
                p.y = randomInt(1, settings_.height - 2);
                ++tries;
            } while ((containsPoint(snake_, p) || containsPoint(obstacles_, p)) && tries < 100);
            if (!containsPoint(snake_, p) && !containsPoint(obstacles_, p)) {
                obstacles_.push_back(p);
            }
        }
    }

    void placeFood() {
        Point p{};
        int tries = 0;
        do {
            p.x = randomInt(1, settings_.width - 2);
            p.y = randomInt(1, settings_.height - 2);
            ++tries;
        } while ((containsPoint(snake_, p) || containsPoint(obstacles_, p) ||
                  (rareFood_.active && pointsEqual(rareFood_.pos, p))) &&
                 tries < 200);
        food_ = p;
    }

    void maybeSpawnRareFood() {
        if (rareFood_.active) return;
        // 以较小概率生成稀有食物
        if (randomInt(0, 99) < 3) {  // 3% 几率
            Point p{};
            int tries = 0;
            do {
                p.x = randomInt(1, settings_.width - 2);
                p.y = randomInt(1, settings_.height - 2);
                ++tries;
            } while ((containsPoint(snake_, p) || containsPoint(obstacles_, p) ||
                      pointsEqual(food_, p)) &&
                     tries < 200);
            if (!containsPoint(snake_, p) && !containsPoint(obstacles_, p) &&
                !pointsEqual(food_, p)) {
                rareFood_.pos = p;
                rareFood_.active = true;
                rareFood_.remainingTicks = 80;  // 存活 80 帧左右
            }
        }
    }

    // ---------- 渲染 ----------
    void draw() {
        clearScreen();

        // 顶部信息
        std::cout << "分数: " << score_ << "   最高分: " << highScore_
                  << "   等级: " << level_
                  << "   模式: " << (settings_.wallKills ? "撞墙死亡" : "穿墙") << "\n";
        std::cout << "操作: WASD / 方向键 移动, P 暂停, Q 结束当前局\n\n";

        // 绘制地图（包含边框）
        for (int y = 0; y < settings_.height; ++y) {
            for (int x = 0; x < settings_.width; ++x) {
                if (y == 0 || y == settings_.height - 1 ||
                    x == 0 || x == settings_.width - 1) {
                    std::cout << '#';
                    continue;
                }

                Point p{x, y};

                if (pointsEqual(p, snake_.front())) {
                    std::cout << '@';  // 头
                } else if (containsPoint(snake_, p)) {
                    std::cout << 'o';  // 身体
                } else if (pointsEqual(p, food_)) {
                    std::cout << '*';  // 普通食物
                } else if (rareFood_.active && pointsEqual(p, rareFood_.pos)) {
                    std::cout << '$';  // 稀有食物
                } else if (containsPoint(obstacles_, p)) {
                    std::cout << 'X';  // 障碍物
                } else {
                    std::cout << ' ';
                }
            }
            std::cout << '\n';
        }

        if (paused_) {
            std::cout << "\n=== 暂停中，按 P 继续 ===\n";
        }
    }

    // ---------- 输入 ----------
    void processInput() {
        if (!_kbhit()) return;

        int ch = _getch();

        // 方向键: 224 + code
        if (ch == 224) {
            int code = _getch();
            if (code == 72) {         // 上
                changeDir(Direction::Up);
            } else if (code == 80) {  // 下
                changeDir(Direction::Down);
            } else if (code == 75) {  // 左
                changeDir(Direction::Left);
            } else if (code == 77) {  // 右
                changeDir(Direction::Right);
            }
            return;
        }

        ch = std::tolower(ch);
        if (ch == 'w') {
            changeDir(Direction::Up);
        } else if (ch == 's') {
            changeDir(Direction::Down);
        } else if (ch == 'a') {
            changeDir(Direction::Left);
        } else if (ch == 'd') {
            changeDir(Direction::Right);
        } else if (ch == 'p') {
            paused_ = !paused_;
        } else if (ch == 'q') {
            // 结束当前局
            running_ = false;
        }
    }

    void changeDir(Direction newDir) {
        // 禁止直接反向
        if ((dir == Direction::Up && newDir == Direction::Down) ||
            (dir == Direction::Down && newDir == Direction::Up) ||
            (dir == Direction::Left && newDir == Direction::Right) ||
            (dir == Direction::Right && newDir == Direction::Left)) {
            return;
        }
        dir_ = newDir;
    }

    // ---------- 更新 ----------
    bool step() {
        if (paused_) return true;

        ticks_++;

        // 计算新头位置
        Point head = snake_.front();
        Point newHead = head;

        if (dir_ == Direction::Up) newHead.y--;
        else if (dir_ == Direction::Down) newHead.y++;
        else if (dir_ == Direction::Left) newHead.x--;
        else if (dir_ == Direction::Right) newHead.x++;

        // 处理边界
        if (settings_.wallKills) {
            if (newHead.x <= 0 || newHead.x >= settings_.width - 1 ||
                newHead.y <= 0 || newHead.y >= settings_.height - 1) {
                return false;
            }
        } else {
            // 穿墙
            if (newHead.x <= 0) newHead.x = settings_.width - 2;
            else if (newHead.x >= settings_.width - 1) newHead.x = 1;
            if (newHead.y <= 0) newHead.y = settings_.height - 2;
            else if (newHead.y >= settings_.height - 1) newHead.y = 1;
        }

        // 撞自己
        if (containsPoint(snake_, newHead)) {
            return false;
        }

        // 撞障碍
        if (containsPoint(obstacles_, newHead)) {
            return false;
        }

        // 移动蛇
        snake_.insert(snake_.begin(), newHead);

        bool grow = false;

        if (pointsEqual(newHead, food_)) {
            // 吃到普通食物
            score_ += 10;
            grow = true;
            placeFood();
        } else if (rareFood_.active && pointsEqual(newHead, rareFood_.pos)) {
            // 吃到稀有食物
            score_ += 40;
            grow = true;
            rareFood_.active = false;
        }

        if (!grow) {
            snake_.pop_back();
        }

        // 稀有食物寿命
        if (rareFood_.active) {
            rareFood_.remainingTicks--;
            if (rareFood_.remainingTicks <= 0) {
                rareFood_.active = false;
            }
        } else {
            // 有机会生成稀有食物
            maybeSpawnRareFood();
        }

        // 根据长度提升等级和速度
        int length = static_cast<int>(snake_.size());
        level_ = std::max(1, 1 + (length - 3) / 5);

        return true;
    }

    int currentDelayMs() const {
        int delay = settings_.baseDelayMs - (level_ - 1) * 8;
        if (delay < 40) delay = 40;
        return delay;
    }

    void gameOver() {
        saveHighScore();
        clearScreen();
        std::cout << "===== 游戏结束 =====\n\n";
        std::cout << "本局得分: " << score_ << "\n";
        std::cout << "历史最高: " << highScore_ << "\n\n";
        std::cout << "1. 再来一局\n";
        std::cout << "2. 返回主菜单\n\n";
        std::cout << "请选择: ";
        std::string choice;
        if (!std::getline(std::cin, choice)) return;

        if (choice == "1") {
            startGame();
        }
        // 选 2 或其它情况都回到主菜单（由上层调用控制）
    }

    void startGame() {
        resetGameState();
        running_ = true;

        while (running_) {
            draw();
            processInput();
            if (!step()) {
                running_ = false;
                break;
            }
            sleepMs(currentDelayMs());
        }

        saveHighScore();
        if (!std::cin.good()) {
            // 如果输入流已坏，简单返回
            return;
        }

        // 只有在不是因为按 Q 主动退出时，才显示「游戏结束」界面
        if (!paused_) {
            gameOver();
        }
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    SnakeGame game;
    game.run();
    return 0;
}


