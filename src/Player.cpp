Player::Player(SDL_Renderer* renderer) : position(0, 0), velocity(0, 0) {
    // 加载玩家纹理
    texture = IMG_LoadTexture(renderer, "assets/sprites/player.png");
    if (!texture) {
        std::cerr << "The player texture failed to load. Using the default red rectangle instead.: " << IMG_GetError() << std::endl;
        // 创建默认红色纹理
        SDL_Surface* surface = SDL_CreateRGBSurface(0, 16, 24, 32, 0, 0, 0, 0);
        SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 0, 0));
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    // 碰撞盒（相对玩家左上角）
    hitbox = {2, 4, 12, 18};
}

Player::~Player() {
    SDL_DestroyTexture(texture);
}

void Player::handleInput() {
    if (dead) return;  // 死亡后不接受输入
    
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    velocity.x = 0;

    if (keys[SDL_SCANCODE_LEFT])  velocity.x = -speed;
    if (keys[SDL_SCANCODE_RIGHT]) velocity.x = speed;

    if ((keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_UP]) && onGround) {
        velocity.y = jumpForce;
        onGround = false;
        std::cout << "Jump! Velocity: " << velocity.y << " pixels/sec" << std::endl;
    }
}

void Player::update(const TiledMap& map, float deltaTime) {
    if (dead) return;  // 死亡后不更新物理
    
    // 应用重力
    velocity.y += gravity * deltaTime;

    // 调试输出物理信息
    static int physicsLogCount = 0;
    if (physicsLogCount++ % 30 == 0) {
        std::cout << "Physics Update - DeltaTime: " << deltaTime 
                  << ", Gravity applied: " << gravity * deltaTime
                  << ", Velocity: (" << velocity.x << ", " << velocity.y << ")" << std::endl;
    }

    // X方向移动与碰撞
    float oldX = position.x;
    position.x += velocity.x * deltaTime;
    
    SDL_Point testPoints[4] = {
        {(int)(position.x + hitbox.x), (int)(position.y + hitbox.y)},
        {(int)(position.x + hitbox.x + hitbox.w - 1), (int)(position.y + hitbox.y)},
        {(int)(position.x + hitbox.x), (int)(position.y + hitbox.y + hitbox.h - 1)},
        {(int)(position.x + hitbox.x + hitbox.w - 1), (int)(position.y + hitbox.y + hitbox.h - 1)}
    };
    
    bool collideX = false;
    for (int i = 0; i < 4; i++) {
        if (map.isColliding(testPoints[i].x, testPoints[i].y)) {
            collideX = true;
            break;
        }
    }
    
    if (collideX) {
        position.x = oldX;
        velocity.x = 0;
    }

    // Y方向移动与碰撞
    float oldY = position.y;
    position.y += velocity.y * deltaTime;
    
    SDL_Point testPointsY[4] = {
        {(int)(position.x + hitbox.x), (int)(position.y + hitbox.y)},
        {(int)(position.x + hitbox.x + hitbox.w - 1), (int)(position.y + hitbox.y)},
        {(int)(position.x + hitbox.x), (int)(position.y + hitbox.y + hitbox.h - 1)},
        {(int)(position.x + hitbox.x + hitbox.w - 1), (int)(position.y + hitbox.y + hitbox.h - 1)}
    };
    
    bool collideY = false;
    bool collideTop = false;
    bool collideBottom = false;
    
    for (int i = 0; i < 4; i++) {
        if (map.isColliding(testPointsY[i].x, testPointsY[i].y)) {
            collideY = true;
            if (i < 2) collideTop = true;
            else collideBottom = true;
        }
    }

    if (collideY) {
        if (collideBottom && velocity.y > 0) {
            onGround = true;
            float worldY = position.y + hitbox.y + hitbox.h;
            int tileY = (int)(worldY) / map.getTileHeight();
            position.y = (tileY * map.getTileHeight()) - hitbox.y - hitbox.h;
            velocity.y = 0;
            std::cout << "Landed on ground" << std::endl;
        } 
        else if (collideTop && velocity.y < 0) {
            position.y = oldY;
            velocity.y = 0;
            std::cout << "Hit ceiling" << std::endl;
        }
    } 
    else 
    {
        onGround = false;
    }

    // 边界检查
    float contentWidth = map.getContentPixelWidth();
    float contentHeight = map.getContentPixelHeight();
    
    if (position.x < 0) position.x = 0;
    if (position.x + 16 > contentWidth) {
        position.x = contentWidth - 16;
    }
    if (position.y < 0) position.y = 0;
    if (position.y + 24 > contentHeight) {
        position.y = contentHeight - 24;
        velocity.y = 0;
        onGround = true;
    }

    // 检查危险物碰撞
    checkCollisionsWithHazards(map);

    // 简化调试输出
    static int outputCount = 0;
    if (outputCount++ % 60 == 0) {
        std::cout << "Player - Pos: (" << position.x << ", " << position.y 
                  << ") Vel: (" << velocity.x << ", " << velocity.y 
                  << ") OnGround: " << onGround 
                  << " Dead: " << dead << std::endl;
    }
}

void Player::checkCollisionsWithHazards(const TiledMap& map) {
    // 检查四个碰撞点是否与危险物重叠
    SDL_Point testPoints[4] = {
        {(int)(position.x + hitbox.x), (int)(position.y + hitbox.y)},
        {(int)(position.x + hitbox.x + hitbox.w - 1), (int)(position.y + hitbox.y)},
        {(int)(position.x + hitbox.x), (int)(position.y + hitbox.y + hitbox.h - 1)},
        {(int)(position.x + hitbox.x + hitbox.w - 1), (int)(position.y + hitbox.y + hitbox.h - 1)}
    };
    
    for (int i = 0; i < 4; i++) {
        if (map.isHazard(testPoints[i].x, testPoints[i].y)) {
            std::cout << "Player died! Hazard collision at point " << i 
                      << " (" << testPoints[i].x << ", " << testPoints[i].y << ")" << std::endl;
            kill();
            return;
        }
    }
}

void Player::render(SDL_Renderer* renderer, const Camera& camera, float renderScale) {
    if (dead) {
        // 死亡时可以显示特殊效果，比如半透明
        SDL_SetTextureAlphaMod(texture, 128);  // 半透明
    }
    
    const SDL_Rect& view = camera.getView();
    SDL_Rect dest = {
        (int)((position.x - view.x) * renderScale),
        (int)((position.y - view.y) * renderScale),
        (int)(16 * renderScale),
        (int)(24 * renderScale)
    };
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
    
    if (dead) {
        SDL_SetTextureAlphaMod(texture, 255);  // 恢复不透明
    }
}
