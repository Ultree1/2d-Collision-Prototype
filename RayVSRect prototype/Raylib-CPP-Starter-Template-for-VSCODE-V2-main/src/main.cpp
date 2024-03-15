#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <vector>

Camera2D originCam;

// a raycasting function returns a ray. a ray's attributes are:
//if it has intersected with a rectangle or not, (collided)
//the coordinates where it intersects the rectangle, the direction of the x and y normals from the collision, (contact_point, contact_normal)
//and the ratio of the shortest ray it would take to collide with the rectangle given its current direction to the ray's actual length. (rayCheck)
//if rayCheck is below 1 AND collided is true, a collision has occured.

struct ray {
    bool collided;
    Vector2 contact_point, contact_normal;
    float rayCheck;
};

//a collision function should return zeroRay when it knows a collision will not take place given the input parameters
ray zeroRay = {0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

struct movingRect{
    Vector2 position;
    Vector2 size;
    Vector2 velocity;
    Vector2 acc;
};

//returns a ray struct after being given an origin, direction, and a rectangle to collide with.
ray RayVsRect(const Vector2& ray_origin, const Vector2& ray_dir, movingRect r){

Vector2 t_near = Vector2Divide((Vector2Subtract(r.position, ray_origin)), ray_dir);
Vector2 t_far =  Vector2Divide(Vector2Add(r.position, Vector2Subtract(r.size, ray_origin)), ray_dir);

if(std::isnan(t_far.y) || std::isnan(t_far.x)) return zeroRay;
if(std::isnan(t_near.y) || std::isnan(t_near.x)) return zeroRay;

if(t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
if(t_near.y > t_far.y) std::swap(t_near.y, t_far.y);

if(t_near.x > t_far.y || t_near.y > t_far.x) return zeroRay;

float t_hit_near = std::max(t_near.x, t_near.y);
float t_hit_far = std::min(t_far.x, t_far.y);

if (t_hit_far < 0) return zeroRay;

Vector2 contact_point = Vector2 {std::round(Vector2Add(ray_origin, Vector2Multiply(Vector2 {t_hit_near, t_hit_near}, ray_dir)).x), std::round(Vector2Add(ray_origin, Vector2Multiply(Vector2 {t_hit_near, t_hit_near}, ray_dir)).y)}; 
Vector2 contact_normal = {0, 0};

if(t_near.x > t_near.y){
    if(ray_dir.x < 0) contact_normal = {1, 0};
    else contact_normal = {-1, 0};
}
else if (t_near.x < t_near.y){
    if (ray_dir.y < 0) contact_normal = {0, 1};
    else contact_normal = {0, -1};
}

DrawText(TextFormat("tHitNear = %f x = %f y = %f, \n \n normX = %f, normY = %f", t_hit_near, contact_point.x, contact_point.y, contact_normal.x, contact_normal.y), 0, 0, 32, WHITE);
return ray {1, contact_point, contact_normal, t_hit_near};
}

//returns a ray struct when given two rectangles, the "in" rectangle should be considered the moving one, and the "target" rectangle should be static (not moving).
//The DynamicRectVSRect function calls the rayVsRect function. The ray's origin is the 'in' rectangle's center coordinates, and the ray direction is the 'in' rectangle's velocity modulated by deltaTime.
//The single rectangle input for RayVsRect should be the 'target' rectangle expanded by half the width and height of the 'in' rectangle.
ray DynamicRectVSRect(const movingRect& in, const movingRect& target){
    if(in.velocity.x == 0 && in.velocity.y == 0) return zeroRay;

    movingRect expanded_target;
    expanded_target.position.x = target.position.x - in.size.x/2;
    expanded_target.position.y = target.position.y - in.size.y/2;
    expanded_target.size.x = target.size.x + in.size.x;
    expanded_target.size.y = target.size.y + in.size.y;
    
    Vector2 inCenter = {in.position.x + in.size.x/2, in.position.y + in.size.y/2};

    ray RectRay = RayVsRect(inCenter, Vector2{in.velocity.x*GetFrameTime(), in.velocity.y*GetFrameTime()}, expanded_target);
    if(RectRay.collided && RectRay.rayCheck <= 1.0f) {
        return RectRay;
    }
    else return zeroRay;
    }

//this vector stores each rectangle for easy drawing and collision detection purposes
std::vector<movingRect> vRects;

void SetupGame(){
    // First rectangle in this list is always the 'player rectangle'
    // and is always controlled by the mouse.

    vRects.push_back(movingRect {10.0f, 10.0f, 30.0f, 20.0f});

    vRects.push_back(movingRect {300.0f, 200.0f, 300.0f, 200.0f});
    vRects.push_back(movingRect {610.0f, 200.0f, 10.0f, 10.0f});
    vRects.push_back(movingRect {610.0f, 180.0f, 10.0f, 10.0f});
    vRects.push_back(movingRect {610.0f, 160.0f, 10.0f, 10.0f});
    vRects.push_back(movingRect {610.0f, 140.0f, 10.0f, 10.0f});

    vRects.push_back(movingRect {120.0f, 700.0f, 80.0f, 50.0f});
    vRects.push_back(movingRect {200.0f, 700.0f, 80.0f, 50.0f});
    vRects.push_back(movingRect {280.0f, 700.0f, 80.0f, 50.0f});
    vRects.push_back(movingRect {360.0f, 700.0f, 80.0f, 50.0f});
    vRects.push_back(movingRect {440.0f, 700.0f, 80.0f, 50.0f});
    vRects.push_back(movingRect {520.0f, 700.0f, 80.0f, 50.0f});
    vRects.push_back(movingRect {600.0f, 700.0f, 80.0f, 50.0f});
    vRects.push_back(movingRect {680.0f, 700.0f, 80.0f, 50.0f});
    vRects.push_back(movingRect {760.0f, 700.0f, 80.0f, 50.0f});
    vRects.push_back(movingRect {840.0f, 700.0f, 80.0f, 50.0f});
    vRects.push_back(movingRect {840.0f, 650.0f, 80.0f, 50.0f});
    vRects.push_back(movingRect {840.0f, 600.0f, 80.0f, 50.0f});
    vRects.push_back(movingRect {840.0f, 550.0f, 80.0f, 50.0f});
    vRects.push_back(movingRect {840.0f, 500.0f, 80.0f, 50.0f});

}


void MoveCamera()
{

BeginMode2D(originCam);

}

bool drawingRectangle = false;
Vector2 RectangleOrigin;

void GetInput() {
//Press the right mouse button twice in different areas of the screen to create a new rectangle.
if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
    
    if(drawingRectangle){

    if(RectangleOrigin.x > GetMousePosition().x)
    {
        if(RectangleOrigin.y > GetMousePosition().y){
           vRects.push_back(movingRect {GetMousePosition().x, GetMousePosition().y, RectangleOrigin.x - GetMousePosition().x, RectangleOrigin.y - GetMousePosition().y});
        }
        else
        {
            vRects.push_back(movingRect {GetMousePosition().x, RectangleOrigin.y, RectangleOrigin.x - GetMousePosition().x, GetMousePosition().y-RectangleOrigin.y});
        }
    }
      if(RectangleOrigin.x <= GetMousePosition().x)
    {
        if(RectangleOrigin.y > GetMousePosition().y){
            vRects.push_back(movingRect {RectangleOrigin.x, GetMousePosition().y, GetMousePosition().x - RectangleOrigin.x, RectangleOrigin.y - GetMousePosition().y});
        }
        else
        {
            vRects.push_back(movingRect {RectangleOrigin.x, RectangleOrigin.y, GetMousePosition().x - RectangleOrigin.x, GetMousePosition().y-RectangleOrigin.y});
        }
    }
    drawingRectangle = false;
    }
    else{
    RectangleOrigin = GetMousePosition();
    drawingRectangle = true;
    }
}

}

void RunLogic() {

//the commented code below was for testing my raycasting implementation, 
//it draws a line from 200, 200 to your cursor and draws a circle where the line intersects the first rectangle in the vRects vector.

/*Vector2 ray_point = {200.0f, 200.0f};
Vector2 ray_direction = Vector2Subtract(GetMousePosition(), ray_point);

DrawLine(ray_point.x, ray_point.y, GetMousePosition().x, GetMousePosition().y, YELLOW);
ray PlayerRay = RayVsRect(ray_point, ray_direction, vRects[0]);

if(PlayerRay.collided && PlayerRay.rayCheck <= 1)
{
    DrawRectangle(vRects[0].position.x, vRects[0].position.y, vRects[0].size.x, vRects[0].size.y, YELLOW);
    DrawCircle(PlayerRay.contact_point.x, PlayerRay.contact_point.y, 5, RED);
    DrawLine(PlayerRay.contact_point.x, PlayerRay.contact_point.y, PlayerRay.contact_point.x+5*PlayerRay.contact_normal.x, PlayerRay.contact_point.y + 5*PlayerRay.contact_normal.y, YELLOW);
    
}
else {
    DrawRectangle(vRects[0].position.x, vRects[0].position.y, vRects[0].size.x, vRects[0].size.y, WHITE);
}
*/


//Press the left mouse button to make the player rectangle accelerate towards your cursor.
//Press the "R" Key in order to reset the player's position and velocity to zero.
Vector2 rayPoint = {vRects[0].position.x, vRects[0].position.y};
Vector2 rayDirection = {Vector2Subtract(GetMousePosition(), rayPoint)};
if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
    vRects[0].velocity.x += Vector2Normalize(rayDirection).x * 200.0f * GetFrameTime();
    vRects[0].velocity.y += Vector2Normalize(rayDirection).y * 200.0f * GetFrameTime();
}
if(IsKeyPressed(KEY_R)){
    vRects[0].position.x = 0; 
    vRects[0].position.y = 0;
    vRects[0].velocity.x = 0;
    vRects[0].velocity.y = 0;
}

DrawText(TextFormat("X = %f, Y = %f, VelX = %f, VelY = %f,", vRects[0].position.x, vRects[0].position.y, vRects[0].velocity.x, vRects[0].velocity.y), 300, 500, 20, WHITE);

std::vector<std::pair<int, float>> z;

for(int i = 1; i < int(vRects.size()); i++)
{ 
    ray RectRay = DynamicRectVSRect(vRects[0], vRects[i]);
    if(RectRay.collided && RectRay.rayCheck <= 1.0f){
        z.push_back({i, RectRay.rayCheck});
    }
}

//This should theoretically sort the collisions by shortest to longest, then resolve the shortest collision. If i screwed up then please tell me!
std::sort(z.begin(), z.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b)
{
    return a.second < b.second;
});

for (auto j : z)
if (DynamicRectVSRect(vRects[0], vRects[j.first]).collided)
{
    ray RectRay = DynamicRectVSRect(vRects[0], vRects[j.first]);
    //The collision is resolved by truncating the velocity to the point where the moving rectangle can never intersect with the static rectangle
    //I also added a one-pixel buffer around the moving rectangle, as there were some issues with the origin of the raycast being from inside the static rectangle when the pixel buffer was removed.
vRects[0].velocity = Vector2Add(Vector2Add(vRects[0].velocity, Vector2{RectRay.contact_normal.x, RectRay.contact_normal.y}), Vector2Multiply(RectRay.contact_normal, Vector2Scale((Vector2){fabsf(vRects[0].velocity.x), fabsf(vRects[0].velocity.y)}, (1-RectRay.rayCheck))));
}

//change the moving rectangle's position by its velocity modulated by deltaTime
vRects[0].position.x += vRects[0].velocity.x * GetFrameTime();
vRects[0].position.y += vRects[0].velocity.y * GetFrameTime();
}

void DrawGame(){

//automatically draws each rectangle in the vRects vector, plus a one pixel expansion to make up for the one-pixel buffer i added to the player.
//if there is a more elegant way for this to work, please tell me.
for(const auto& r : vRects){
    DrawRectangleLines(r.position.x, r.position.y, r.size.x+1, r.size.y+1, WHITE);
}


}

int main()
{

    const int screenWidth = 1280;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "2d Collision Prototype");
    SetTargetFPS(60);
    SetupGame();
    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(BLACK);
        GetInput();
        RunLogic();
        DrawGame();
        EndDrawing();

    }

    CloseWindow();
    return 0;
}