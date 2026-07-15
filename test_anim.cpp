#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void getCardProps(float dt, int& x, int& y, int& w, int& h, float& scale) {
    float adt = std::abs(dt);
    float t;
    float cx, fW, fH, fY;
    if (adt <= 1.0f) {
        t = adt;
        cx = lerp(64, 96, t);
        fW = lerp(36, 24, t);
        fH = lerp(48, 38, t);
        fY = lerp(15, 20, t);
        scale = lerp(1.0f, 0.75f, t);
    } else if (adt <= 2.0f) {
        t = adt - 1.0f;
        cx = lerp(96, 119, t);
        fW = lerp(24, 18, t);
        fH = lerp(38, 28, t);
        fY = lerp(20, 25, t);
        scale = lerp(0.75f, 0.5f, t);
    } else if (adt <= 3.0f) {
        t = adt - 2.0f;
        cx = lerp(119, 138, t);
        fW = lerp(18, 0, t);
        fH = lerp(28, 0, t);
        fY = lerp(25, 32, t);
        scale = lerp(0.5f, 0.0f, t);
    } else {
        w = 0; h = 0; return;
    }
    
    if (dt < 0) {
        cx = 64 - (cx - 64);
    }
    
    w = (int)fW;
    h = (int)fH;
    x = (int)(cx - w / 2.0f);
    y = (int)fY;
}

int main() {
    float anim_pos = 0.5f; // Animating from 0 to 1
    int total = 5;
    
    struct CardAnim {
        int idx;
        float dt;
        float adt;
    };
    std::vector<CardAnim> cards;
    
    for (int i = 0; i < total; i++) {
        float dt = i - anim_pos;
        while (dt > total / 2.0f) dt -= total;
        while (dt < -total / 2.0f) dt += total;
        
        if (std::abs(dt) < 3.0f) {
            cards.push_back({i, dt, std::abs(dt)});
        }
    }
    
    std::sort(cards.begin(), cards.end(), [](const CardAnim& a, const CardAnim& b) {
        return a.adt > b.adt;
    });
    
    for (auto c : cards) {
        int x, y, w, h;
        float scale;
        getCardProps(c.dt, x, y, w, h, scale);
        std::cout << "Idx " << c.idx << " (dt " << c.dt << "): x=" << x << " y=" << y << " w=" << w << " h=" << h << " scale=" << scale << std::endl;
    }
    return 0;
}
