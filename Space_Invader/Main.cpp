#include <array>
#include <chrono>
#include <random>
#include <iostream>
#include <vector>
#include <string>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;

struct Bullet {
    Sprite sprite;
    Vector2f velocity;
    bool Active;
    bool PlayerOrigin;

    Bullet(Vector2f position, Vector2f velocity, const Texture& texture, bool PlayerOrigin, const Color& color = Color::White)
        : velocity(velocity), Active(true), PlayerOrigin(PlayerOrigin) {
        sprite.setTexture(texture);
        sprite.setPosition(position + Vector2f(17.f, 0.f));
        sprite.setColor(color);
    }

    void update(float deltaTime) {
        if (Active) {
            sprite.move(velocity * deltaTime);
        }
    }
};

struct Enemy {
    Sprite sprite;
    Vector2f velocity;
    bool Active, movever;
    int direction, id, flip;

    Enemy(Vector2f position, Vector2f velocity, const Texture& texture, int direction, int id)
        : velocity(velocity), Active(true), direction(direction), movever(false), id(id), flip(1) {
        sprite.setTexture(texture);
        sprite.setPosition(position);
    }

    void update(float deltaTime) {
        if (Active) {
            sprite.move(velocity * deltaTime);
        }
    }

    FloatRect getHitbox() const {
        FloatRect originalBounds = sprite.getGlobalBounds();
        
        //Making the hitbox smaller
        float xOffset = 13.0f;
        float yOffset = 19.0f;

        return FloatRect(
            originalBounds.left + xOffset,
            originalBounds.top + yOffset,
            originalBounds.width - 2 * xOffset,
            originalBounds.height - 2 * yOffset
        );
    }
    void updateTexture(const Texture& newTexture) {
        sprite.setTexture(newTexture);
    }
};

struct TextDisplay {
    Text text;
    Font font;

    TextDisplay(const string& content, int characterSize, Vector2f position)
        : text(content, font), font() {
        font.loadFromFile("Resources/PressStart2P-Regular.ttf");

        text.setFont(font);
        text.setCharacterSize(characterSize);
        text.setFillColor(Color::White);
        text.setPosition(position);
    }
    void draw(RenderWindow& window) {
        window.draw(text);
    }
    void update(const string& newtext) {
        text.setString(newtext);
    }
};

struct Animation {
    Sprite sprite;
    Vector2u frameSize;
    int Maxframes;
    int frame, updatetimer, speed;
    bool Active, loop;

    Animation(Vector2f position, const Texture& texture, Vector2u frameSize, int updatetimer, const bool& loop = false)
        : Maxframes(texture.getSize().x / frameSize.x), frameSize(frameSize), frame(0), updatetimer(updatetimer), Active(true), loop(loop) {
        speed = updatetimer;
        sprite.setTexture(texture);
        sprite.setTextureRect(IntRect(0, 0, frameSize.x, frameSize.y));
        sprite.setPosition(position);
    }

    void update() {
        if (speed == 0) {
            frame = (frame + 1) % Maxframes;
            sprite.setTextureRect(IntRect(frameSize.x * frame, 0, frameSize.x, frameSize.y));
            if (frame >= Maxframes - 1 && !loop)
                Active = false;
            speed = updatetimer;
        }
        else
            speed--;
    }
};


struct Player
{
    Animation* animation;
    Vector2f velocity;
    int direction;
    Player(Vector2f position, const Texture& texture, Vector2u(frameSize))
        : velocity(0.f, 0.f), direction(0) {
        animation = new Animation(position, texture, frameSize, 90, true);
        animation->sprite.setPosition(position);
    }

    void setAnimation(Animation* NewAnimation) {
        delete animation;
        animation = NewAnimation;
    }

    void move(float deltaTime) {
        animation->sprite.move(velocity * deltaTime);
    }

    void update() {
        animation->update();
    }

    //No modified hitbox for the player because they don't deserve any mercy >:)
    void draw(RenderWindow& window) {
        window.draw(animation->sprite);
    }

    ~Player() {
        delete animation;
    }
};

void set_level(RenderWindow& window, Texture enemy_texture, int level) {
    ;
}

int main()
{
    const int Width = 720;
    const int Height = 720;
    bool game_over = 0;
    chrono::microseconds time(0);

    Event event;

    mt19937_64 randomizer(chrono::system_clock::now().time_since_epoch().count());

    RenderWindow window(VideoMode(Width, Height), "Space Invaders");
    window.setView(View(FloatRect(0, 0, Width, Height)));

    Texture Player_texture;
    Player_texture.loadFromFile("Resources/Images/player_animation.png");

    Texture Player_texture_left;
    Player_texture_left.loadFromFile("Resources/Images/Player_Turning_Animation_Left.png");

    Texture Player_texture_right;
    Player_texture_right.loadFromFile("Resources/Images/Player_Turning_Animation_Right.png");

    Texture lives_texture;
    lives_texture.loadFromFile("Resources/Images/Lives.png");
    Sprite lives_display(lives_texture);

    Player player(Vector2f(375.f, 550.f), Player_texture, Vector2u(50, 34));

    TextDisplay pressExit("Press Enter To Exit!", 30, Vector2f(10.f, 90.f));
    TextDisplay GAMEOVER("GAME OVER!", 50, Vector2f(10.f, 10.f));
    TextDisplay YOUWIN("YOU WIN!", 50, Vector2f(10.f, 10.f)); //fml
    TextDisplay Menu_Start("Start Game", 30, Vector2f(200.f, 200.f));
    TextDisplay Menu_Exit("Exit Game", 30, Vector2f(200.f, 300.f));
    TextDisplay Menu_Credit("Credits", 30, Vector2f(200.f, 400.f));

    Texture Explosion_Texture;
    Explosion_Texture.loadFromFile("Resources/Images/Explosion.png");

    Texture Explosion_Texture_small;
    Explosion_Texture_small.loadFromFile("Resources/Images/Explosion_small.png");

    Texture background_texture;
    background_texture.loadFromFile("Resources/Images/Background.png");
    Sprite background_sprite;
    background_sprite.setTexture(background_texture);
    background_sprite.setPosition(0.f, 0.f);

    Texture PlayerBullet;
    PlayerBullet.loadFromFile("Resources/Images/PlayerBullet.png");

    Texture EnemyBullet;
    EnemyBullet.loadFromFile("Resources/Images/EnemyBullet.png");

    Texture enemyBullet1;
    enemyBullet1.loadFromFile("Resources/Images/PlayerBullet.png");

    vector<Enemy> enemies;

    Texture MenuChoice;
    MenuChoice.loadFromFile("Resources/Images/Menu_Choice.png");
    Sprite MenuChoicesprite;
    MenuChoicesprite.setTexture(MenuChoice);
    MenuChoicesprite.setPosition(Vector2f(150.f, 187.f));

    Texture enemyTexture1;
    enemyTexture1.loadFromFile("Resources/Images/Enemy1.png");

    Texture enemyTexture2;
    enemyTexture2.loadFromFile("Resources/Images/Enemy2.png");

    Texture enemyTexture3;
    enemyTexture3.loadFromFile("Resources/Images/Enemy3.png");

    Texture enemyTexture1_1;
    enemyTexture1_1.loadFromFile("Resources/Images/Enemy1_1.png");

    Texture enemyTexture2_1;
    enemyTexture2_1.loadFromFile("Resources/Images/Enemy2_1.png");

    Texture enemyTexture3_1;
    enemyTexture3_1.loadFromFile("Resources/Images/Enemy3_1.png");

    //Spawning enemies... At least attempting to... Dear god I need sleep
    for (int i = 0; i < 5; i++) {
        for (int j = 0, tempdirection; j < 11; j++) {
            Vector2f position = Vector2f(100.0f, 100.0f) + Vector2f(j * 50.0f, i * 50.0f);
            if (i % 2 == 0) {
                tempdirection = 1;
            }
            else {
                tempdirection = -1;
            }
            if (i == 0)
                enemies.emplace_back(position, Vector2f(0.0f, 0.0f), enemyTexture1, tempdirection, 1);
            else if (i == 1 || i == 2)
                enemies.emplace_back(position, Vector2f(0.0f, 0.0f), enemyTexture2, tempdirection, 2);
            else
                enemies.emplace_back(position, Vector2f(0.0f, 0.0f), enemyTexture3, tempdirection, 3);
        }
    }

    int Reloading = 0, enemymoving = 0, difficulty = 1, score = 0, lives = 3, respawn = 0, game_win = 0, game_start = 1, menu_choice = 1, invulnarablity = 0, invtimer = 0;
    vector<Bullet> bullets;
    vector<Animation> animations;
    Vector2f temp_position;

    TextDisplay Lives("Lives: ", 24, Vector2f(10.f, 620.f));
    lives_display.setPosition(Vector2f(150.f, 618.f));
    
    TextDisplay Score_Display("Score: " + to_string(score), 24, Vector2f(10.f, 10.f));

    //Setting frame control and frame duration for 60 FPS
    //Clock Main_clock;
    //Time Game_frameDuration = seconds(1.0f / 60.0f);

    while (window.isOpen()) {
        //Process events
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
        }
        //Time elapsed = Main_clock.restart();

        if (!game_over && !game_win && !game_start) {
            player.velocity.x = 0.f;
            temp_position = player.animation->sprite.getPosition();

            if (Keyboard::isKeyPressed(Keyboard::Left) && player.animation->sprite.getPosition().x > 40) {
                player.velocity.x = -20.f;
                if (player.direction != 1) {
                    player.setAnimation(new Animation(temp_position, Player_texture_left, Vector2u(42, 34), 90, true));
                    player.direction = 1;
                }
            }
            else if (Keyboard::isKeyPressed(Keyboard::Right) && player.animation->sprite.getPosition().x < 640) {
                player.velocity.x = 20.f;
                if (player.direction != 2) {
                    player.setAnimation(new Animation(temp_position, Player_texture_right, Vector2u(42, 34), 90, true));
                    player.direction = 2;
                }
            }
            else {
                if (player.direction != 0) {
                    player.setAnimation(new Animation(temp_position, Player_texture, Vector2u(50, 34), 90, true));
                    player.direction = 0;
                }
            }

            if (Reloading == 0) {
                if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::Z)) {
                    //Create a new bullet and add it to the vector
                    bullets.emplace_back(player.animation->sprite.getPosition(), Vector2f(0.f, -30.f), PlayerBullet, true, Color::Green);
                    sounds.emplace_back(Fire1);
                    Reloading = 1000; //Very dumb implementation but it's 3 am it'll have to do
                }
            }
            else
                Reloading--;
            //Update stuff
            player.move(0.016f);  //Assuming 60fps

            for (auto& bullet : bullets) {
                bullet.update(0.016f);
                //Try to remove bullets that are out of bounds
                if (bullet.sprite.getPosition().y < 0)
                    bullet.Active = false;
            }

            bernoulli_distribution distribution(0.00001);

            if (respawn != 0) {
                respawn--;
                if (respawn == 0)
                    player.animation->sprite.setPosition(375.f, 550.f);
            }

            //Omg enemy shooting who tf gave them a gun O_o
            for (auto& enemy : enemies) {
                enemy.update(0.016f);
                if (distribution(randomizer)) {
                    if (enemy.id == 1) {
                        bullets.emplace_back(enemy.sprite.getPosition() + Vector2f(-15.0f, 0.f), Vector2f(0.f, 20.f), EnemyBullet, false);
                        bullets.emplace_back(enemy.sprite.getPosition() + Vector2f(0.f, 20.f), Vector2f(0.f, 20.f), EnemyBullet, false);
                        bullets.emplace_back(enemy.sprite.getPosition() + Vector2f(15.0f, 0.f), Vector2f(0.f, 20.f), EnemyBullet, false);
                    }
                    if (enemy.id == 2)
                        bullets.emplace_back(enemy.sprite.getPosition(), Vector2f(0.f, 30.f), EnemyBullet, false, Color::Yellow);
                    if (enemy.id == 3) {
                        bullets.emplace_back(enemy.sprite.getPosition(), Vector2f(2.f, 20.f), PlayerBullet, false, Color::Red);
                        bullets.emplace_back(enemy.sprite.getPosition(), Vector2f(-2.f, 20.f), PlayerBullet, false, Color::Red);
                    }
                }
            }

            for (auto& animation : animations) {
                animation.update();
            }

            if (enemymoving == 0) {
                for (auto& enemy : enemies) {
                    if (enemy.movever == false) {
                        if ((enemy.sprite.getPosition().x == 640 || enemy.sprite.getPosition().x == 40)) {
                            enemy.sprite.setPosition(enemy.sprite.getPosition() + Vector2f(0.0f, 5.0f));
                            enemy.direction *= -1;
                            enemy.movever = true;
                        }
                        else
                            enemy.sprite.setPosition(enemy.sprite.getPosition() + Vector2f(5.0f * enemy.direction, 0.0f));
                    }
                    else if (enemy.movever == true) {
                        if (int(enemy.sprite.getPosition().y) % 50 == 0) {
                            enemy.sprite.setPosition(enemy.sprite.getPosition() + Vector2f(5.0f * enemy.direction, 0.0f));
                            enemy.movever = false;
                        }
                        else {
                            enemy.sprite.setPosition(enemy.sprite.getPosition() + Vector2f(0.0f, 5.0f));
                            if (enemy.sprite.getPosition().y == 550.f)
                                game_over = 1;
                        }
                    }
                    if (enemy.id == 1) {
                        if (enemy.flip)
                            enemy.updateTexture(enemyTexture1_1);
                        else
                            enemy.updateTexture(enemyTexture1);
                        enemy.flip = !enemy.flip;
                    }
                    else if (enemy.id == 2) {
                        if (enemy.flip)
                            enemy.updateTexture(enemyTexture2_1);
                        else
                            enemy.updateTexture(enemyTexture2);
                        enemy.flip = !enemy.flip;
                    }
                    else {
                        if (enemy.flip)
                            enemy.updateTexture(enemyTexture3_1);
                        else
                            enemy.updateTexture(enemyTexture3);
                        enemy.flip = !enemy.flip;
                    }
                }
                enemymoving = 1000 - difficulty; //Reset the last move time
            }
            else
                enemymoving--;

            for (auto& bullet : bullets) {
                for (auto& enemy : enemies) {
                    if (bullet.sprite.getGlobalBounds().intersects(enemy.getHitbox()) && bullet.PlayerOrigin == true) {
                        //Collision detected, remove both the bullet and the enemy
                        bullet.Active = false;
                        enemy.Active = false;
                        animations.emplace_back(enemy.sprite.getPosition() + Vector2f(13.f, 13.f), Explosion_Texture_small, Vector2u(25, 25), 30);
                        score++;
                        Score_Display.update("Score: " + to_string(score));
                        if (score == 55)
                            game_win = 1;
                        difficulty = min(975, 40 * score / 2);
                    }
                }
                if (bullet.sprite.getGlobalBounds().intersects(player.animation->sprite.getGlobalBounds()) && bullet.PlayerOrigin == false) {
                    if (invulnarablity == 0) {
                        lives--;
                        bullet.Active = false;
                        if (player.direction == 0)
                            animations.emplace_back(player.animation->sprite.getPosition() + Vector2f(0.f, -12.f), Explosion_Texture, Vector2u(50, 50), 30); //perhaps?
                        else
                            animations.emplace_back(player.animation->sprite.getPosition() + Vector2f(8.f, -12.f), Explosion_Texture, Vector2u(50, 50), 30);
                        invulnarablity = 300;
                        player.animation->sprite.setPosition(375.f, -100.f);
                        respawn = 1000;
                        if (lives == 0)
                            game_over = 1;
                    }
                }
            }

            enemies.erase(remove_if(enemies.begin(), enemies.end(), [](const Enemy& enemy) { return !enemy.Active; }), enemies.end());
            bullets.erase(remove_if(bullets.begin(), bullets.end(), [](const Bullet& bullet) { return !bullet.Active; }), bullets.end());
            animations.erase(remove_if(animations.begin(), animations.end(), [](const Animation& animation) { return !animation.Active; }), animations.end());

            //Clear the window
            window.clear();

            //Draw
            window.draw(background_sprite);
            if (invulnarablity <= 0)
                player.draw(window);
            else {
                if (invtimer == 0) {
                    player.draw(window);
                    invtimer = 10;
                    invulnarablity--;
                }
                invtimer--;
            }

            Lives.draw(window);
            Score_Display.draw(window);
            if (lives == 2)
                lives_display.setTextureRect(IntRect(0, 0, 100, 50));
            if (lives == 1)
                lives_display.setTextureRect(IntRect(0, 0, 50, 50));
                
            window.draw(lives_display);
            //Draw enemies
            for (const auto& enemy : enemies) {
                window.draw(enemy.sprite);
            }
            for (const auto& bullet : bullets) {
                window.draw(bullet.sprite);
            }
            for (const auto& animation : animations) {
                window.draw(animation.sprite);
            }
            player.animation->update();
            window.display();
        }
        else {
            window.clear();

            window.draw(background_sprite);

            if (Keyboard::isKeyPressed(Keyboard::Enter)) {
                if (menu_choice == 1)
                    game_start = 0;
                else if (menu_choice == 2)
                    window.close();
                else {

                }
            }
            //Draw
            if (game_over) {
                GAMEOVER.draw(window);
                pressExit.draw(window);
                if (Keyboard::isKeyPressed(Keyboard::Enter)) {
                    window.close();
                }
            }
            else if (game_win) {
                YOUWIN.draw(window);
                pressExit.draw(window);
                if (Keyboard::isKeyPressed(Keyboard::Enter)) {
                    window.close();
                }
            }
            else if (game_start) {
                if (Reloading == 0) {
                    if (Keyboard::isKeyPressed(Keyboard::Down)) {
                        if (menu_choice < 3)
                            menu_choice++;
                        else
                            menu_choice = 1;
                    }
                    if (Keyboard::isKeyPressed(Keyboard::Up)) {
                        if (menu_choice > 1)
                            menu_choice--;
                        else
                            menu_choice = 3;
                    }
                    Reloading = 300;
                }
                else
                    Reloading--;
                MenuChoicesprite.setPosition(Vector2f(150.f, 100.f * menu_choice + 87.f));
                window.draw(MenuChoicesprite);
                Menu_Start.draw(window);
                Menu_Exit.draw(window);
                Menu_Credit.draw(window);
            }

            window.display();
        }
        /*
        //FIX THIS FOR THE LOVE OF GOD
        Time sleepTime = Game_frameDuration - elapsed;
        if (operator>(sleepTime, Time::Zero)) {
            sleep(sleepTime);
        }
        */
    }
}