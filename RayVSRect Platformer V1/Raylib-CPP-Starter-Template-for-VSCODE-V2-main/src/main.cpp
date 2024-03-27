#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>
#include "animation.h"
using namespace std;

Camera2D originCam;
Camera2D playerCam;

//Sprite stuff
    Texture2D playerSprite;

// a raycasting function returns a ray. a ray's attributes are:
//if it has intersected with a rectangle or not, (collided)
//the coordinates where it intersects the rectangle, the direction of the x and y normals from the collision, (contact_point, contact_normal)
//and the ratio of the shortest ray it would take to collide with the rectangle given its current direction to the ray's actual length. (rayCheck)
//if rayCheck is below 1 AND collided is true, a collision has occured.

struct ray {
    bool collided;
    Vector2 contact_point, contact_normal;
    float rayCheck;
    int type = 1;
};

struct collision {
    int first;
    float second;
    int third;
};
//a collision function should return zeroRay when it knows a collision will not take place given the input parameters
ray zeroRay = {0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

struct movingRect{
    Vector2 position;
    Vector2 size;
    int type = 1;
    float mass = 1;
    Vector2 velocity;
    Vector2 acc;
    Vector2 force;
    
};


/*std::vector<std::pair<bool*, float>> vTimers;
void timer(bool inputBool, float timeWindow, float timerVar = GetTime()){
    timerVar = GetTime();
    vTimers.push_back({&inputBool, float(timerVar + timeWindow)});
}
*/

float sign(float in){
    if(in < 0) return -1;
    if(in > 0) return 1;
    if(in == 0) return 0;
}


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
//debug raycasting info
//DrawText(TextFormat("tHitNear = %f x = %f y = %f, \n \n normX = %f, normY = %f \n\n type = %i", t_hit_near, contact_point.x, contact_point.y, contact_normal.x, contact_normal.y, r.type), 0, 0, 32, WHITE);
return ray {1, contact_point, contact_normal, t_hit_near, r.type};
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
    RectRay.type = target.type;
    if(RectRay.collided && RectRay.rayCheck <= 1.0f) {
        return RectRay;
    }
    else return zeroRay;
    }

//this vector stores each rectangle for easy drawing and collision detection purposes
std::vector<movingRect> vRects;
std::vector<movingRect> vSpikes;
#define player vRects[0]

void applyForce(float fx, float fy){

player.force = Vector2Add(player.force, Vector2 {fx, fy});

}

void SetupGame(){


    playerSprite = LoadTexture("textures/SealPlayer.png");

    // First rectangle in this list is always the 'player rectangle'
    // and is always controlled by the mouse.

    vRects.push_back(movingRect {10.0f, 10.0f, 31.0f, 31.0f, 0, 2});

    vRects.push_back(movingRect {100.0f, 650.0f, 50.0f, 50.0f, 2, 1});
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

void saveLevel(){
        ofstream inLevel;
    inLevel.open("LevelOne.txt");

    if(inLevel.is_open()){
    for(int i = 0; i < vRects.size(); i++){
        
    inLevel << vRects[i].position.x  << "," << vRects[i].position.y << "," << vRects[i].size.x << "," << vRects[i].size.y << "," << vRects[i].type << endl;
    
    }
}
    inLevel.close();
}

void loadLevel(){
    std::ifstream myfile("LevelOne.txt");
    std::string RectangleData;
    vRects.clear();
    movingRect RectIn;
    while(getline(myfile, RectangleData)){
        int firstpos = 0;
        int commaPos[5] = {0};
        std::string RectString[5];
    for(int i = 1; i < 5; i++){
    firstpos = RectangleData.find(",", std::size_t(firstpos + 1));
    commaPos[i] = firstpos;
    
    }

    RectString[0] = RectangleData.substr(0, commaPos[1]);

    for(int i = 1; i < 5; i++){
        RectString[i] = RectangleData.substr(commaPos[i] + 1, commaPos[i+1] - commaPos[i] - 1);
    }

    cout << commaPos[0] << "," << commaPos[1] << "," << commaPos[2] << "," << commaPos[3] << "{";
    
    for(int i = 0; i < 5; i++){
        cout << RectString[i] << "|";
    }
    cout << "\n";
    RectIn.position.x = std::stoi(RectString[0]);
    RectIn.position.y = std::stoi(RectString[1]);
    RectIn.size.x = std::stoi(RectString[2]);
    RectIn.size.y = std::stoi(RectString[3]);
    RectIn.type = std::stoi(RectString[4]);
    vRects.push_back(RectIn);
    }


}

//camera movement variables
int cameraMode = 1;
Camera2D currentCam = originCam;
float originZoom = 1.0f;
Vector2 originTarget;
void MoveCamera()
{

DrawText(TextFormat("target.x = %f, target.y = %f, camMode = %i", currentCam.target.x, currentCam.target.y, cameraMode ), 100, 300, 20, WHITE);

if(cameraMode == 0){
currentCam = originCam;
originCam.offset = Vector2 {float(GetScreenWidth())/2, float(GetScreenHeight())/2};

if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
    originTarget.x -= GetMouseDelta().x / originZoom;
    originTarget.y -= GetMouseDelta().y / originZoom; 
}

originCam.target = originTarget;
originCam.rotation = 0.0f;
originZoom += GetMouseWheelMove()*0.1;
originCam.zoom = originZoom;

}

else if(cameraMode == 1){
    currentCam = playerCam;
    playerCam.offset = Vector2 {float(GetScreenWidth())/2, float(GetScreenHeight())/2};
    playerCam.zoom = 1.0f;
    playerCam.rotation = 0.0f;
    playerCam.target = Vector2 {player.position.x - player.velocity.x*GetFrameTime(), player.position.y - player.velocity.y*GetFrameTime()};
}


    BeginMode2D(currentCam);

}


//game variables
KeyboardKey KEY_JUMP;
Vector2 playerSpawn = Vector2 {100, 100};
float gravity = 1500;
float gravityModifier = 1;
float playerHeight = 31;
float crouchHeight = 23;
float playerSpeed = 300;
float slideSpeed = 750;
float accConstant = 30;
float brakingConstant = 30;
float walkingVel = 0;
float groundedCoyoteTimer;
float wallslideLeftCoyoteTimer, wallslideRightCoyoteTimer;
float groundedCoyoteWindow = 0.1;
float wallslideCoyoteWindow = 0.2;
bool crouching = 0;
bool sliding = 0;


Vector2 RectangleOrigin;
bool drawingRectangle = false;
float tileSize = 16.0f;
const int worldWidth = 200;
const int worldHeight = 200;
int worldArray[worldWidth][worldHeight] = {0}; 
int RectangleType = 1;
float bufferJumpTimer, noControlTimer;
float bufferWindow = 0.1;
float noControlWindow = 0.20;
bool controlsEnabled = 1;
bool grounded, jumping, wallslidingRight, wallslidingLeft;
float jumpVel = 600;
float wallJumpVel = 420;
float pushoffVel = 400;

void playerDeath() {
player.position = playerSpawn;
player.velocity = Vector2{0,0};
}

void playerJump() {

//debug info
    if(KEY_JUMP == KEY_W){
       DrawText(TextFormat("BJT: %f, Time: %f, KEYJUMP: W", bufferJumpTimer, GetTime()), 100, 100, 20, YELLOW); 
    }
    if(KEY_JUMP == KEY_SPACE){
       DrawText(TextFormat("BJT: %f, Time: %f, KEYJUMP: SPACE", bufferJumpTimer, GetTime()), 100, 100, 20, YELLOW); 
    }



//walljump logic
    if(!grounded)
    {
        if(wallslidingRight || GetTime() < wallslideRightCoyoteTimer)
        {
        controlsEnabled = 0;
        player.velocity.y = -wallJumpVel;
        player.velocity.x = -pushoffVel;
        noControlTimer = GetTime();
        jumping = 1;
        wallslidingRight = 0;
        wallslideRightCoyoteTimer = 0;
        }

        else if (wallslidingLeft || GetTime() < wallslideLeftCoyoteTimer)
        {
        controlsEnabled = 0;
        player.velocity.y = -wallJumpVel;
        player.velocity.x = pushoffVel;
        noControlTimer = GetTime();
        jumping = 1;
        wallslidingLeft = 0;
        wallslideLeftCoyoteTimer = 0;
        }

    else{
    bufferJumpTimer = GetTime();
        }
    }



    if((grounded) || GetTime() < groundedCoyoteTimer ){

        if(sliding){
            brakingConstant = 0;
            player.velocity.x = sign(player.velocity.x) * 600;
            player.velocity.y = -jumpVel;
            grounded = 0;
            jumping = 1;
        }

        else{
    player.velocity.y = -jumpVel;
    grounded = 0;
    jumping = 1;
        }
    }



}


bool gridEnabled = 0;

void GetInput() {

player.force = Vector2 {0, 0};


if(IsKeyDown(KEY_LEFT_CONTROL)){
    if(IsKeyPressed(KEY_S)){
    saveLevel();
    }
    else if(IsKeyPressed(KEY_O)){
        loadLevel();
    }
    else if(IsKeyPressed(KEY_Z)){
        vRects.erase(vRects.end());
    }
}

if(IsKeyPressed(KEY_M)){
    for(int i = 0; i < 1000; i++){
        vRects.push_back(movingRect {10, 10, 10, 10, 2});
    }
}

if(IsKeyPressed(KEY_G) && gridEnabled == 1) gridEnabled = 0;
else if(IsKeyPressed(KEY_G) && gridEnabled == 0) gridEnabled = 1;

if(IsKeyPressed(KEY_ONE)) RectangleType = 1;
else if(IsKeyPressed(KEY_TWO)) RectangleType = 2;

//Press the right mouse button twice in different areas of the screen to create a new rectangle.
if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
    if(!gridEnabled){
        //free draw rectangle (gridless)
    if(drawingRectangle){

        Vector2 RectangleSecondary;
        RectangleSecondary.x = std::roundf(GetScreenToWorld2D(GetMousePosition(), currentCam).x);
        RectangleSecondary.y = std::roundf(GetScreenToWorld2D(GetMousePosition(), currentCam).y);

    if(RectangleOrigin.x > RectangleSecondary.x)
    {
        if(RectangleOrigin.y > RectangleSecondary.y){
           vRects.push_back(movingRect {RectangleSecondary.x, RectangleSecondary.y, RectangleOrigin.x - RectangleSecondary.x, RectangleOrigin.y - RectangleSecondary.y, RectangleType});
        }
        else
        {
            vRects.push_back(movingRect {RectangleSecondary.x, RectangleOrigin.y, RectangleOrigin.x - RectangleSecondary.x, RectangleSecondary.y-RectangleOrigin.y, RectangleType});
        }
    }
      if(RectangleOrigin.x <= RectangleSecondary.x)
    {
        if(RectangleOrigin.y > RectangleSecondary.y){
            vRects.push_back(movingRect {RectangleOrigin.x, RectangleSecondary.y, RectangleSecondary.x - RectangleOrigin.x, RectangleOrigin.y - RectangleSecondary.y, RectangleType});
        }
        else
        {
            vRects.push_back(movingRect {RectangleOrigin.x, RectangleOrigin.y, RectangleSecondary.x - RectangleOrigin.x, RectangleSecondary.y-RectangleOrigin.y, RectangleType});
        }
    }
    drawingRectangle = false;
    }
    else{
    RectangleOrigin = (GetScreenToWorld2D(GetMousePosition(), currentCam));
    RectangleOrigin.x = std::roundf(RectangleOrigin.x);
    RectangleOrigin.y = std::roundf(RectangleOrigin.y);
    drawingRectangle = true;
    }
}
}

if(gridEnabled){
if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
    DrawText(TextFormat("vRects.size() = %i", vRects.size()), 100, 500, 20, WHITE);
    movingRect newRect = movingRect {tileSize*(int(((GetScreenToWorld2D(GetMousePosition(), currentCam)).x)/tileSize)), tileSize*(int(((GetScreenToWorld2D(GetMousePosition(), currentCam)).y)/tileSize)), tileSize, tileSize, RectangleType};
    for(int i = 1; i < int(vRects.size()); i++){
        if(Vector2Equals(newRect.size, vRects[i].size) && Vector2Equals(newRect.position, vRects[i].position)){
            vRects.erase(vRects.begin()+i);
        }

    }

    vRects.push_back(newRect);
}
}


if(GetTime() <= noControlTimer + noControlWindow && GetTime() >= noControlTimer){
    controlsEnabled = 0;
}
else{
    controlsEnabled = 1;    
}

if(controlsEnabled){

if(IsKeyPressed(KEY_R)){
    playerDeath();
}
//camera controls
if(IsKeyPressed(KEY_C)){
    if(cameraMode == 0){
         cameraMode = 1;
         }
    else if(cameraMode == 1) {
        cameraMode = 0;
        }
}


    



//JUMP LOGIC
if(IsKeyPressed(KEY_W)){
    KEY_JUMP = KEY_W;
    playerJump();
    
}

if(IsKeyPressed(KEY_SPACE)){
    KEY_JUMP = KEY_SPACE;
    playerJump();
    
}

if((grounded || wallslidingLeft || wallslidingRight) && GetTime() <= bufferJumpTimer + bufferWindow && GetTime() >= bufferJumpTimer){
playerJump();
bufferJumpTimer = 0;
}

//if(jumping){

    if(IsKeyUp(KEY_JUMP) && jumping){
       //if player is moving up, double the force of gravity until they're not moving up, then apply normal gravity
        if(player.velocity.y < 0){
        gravityModifier = 2.0;
        }       

    }
else {
    gravityModifier = 1;
}
    if(player.velocity.y >= 0){
            gravityModifier = 1;
        }

    if(grounded){
        jumping = false;
        gravityModifier = 1;
        }
//END JUMP LOGIC

//movement logic
float targetSpeed = 0;


if(!crouching){
if(IsKeyDown(KEY_A)){
    targetSpeed = -playerSpeed;
    float speedDif = player.velocity.x - targetSpeed;
    float movement = -(speedDif*accConstant);
    applyForce(movement, 0);
}
else if (IsKeyDown(KEY_D)){
    targetSpeed = playerSpeed;
    float speedDif = player.velocity.x - targetSpeed;
    float movement = -speedDif*accConstant;
    applyForce(movement, 0);
}
else{
    targetSpeed = 0;
    float speedDif = -player.velocity.x;
    float movement = speedDif*brakingConstant;
    applyForce(movement, 0);
}
}

//crouched movement logic
else if(crouching && grounded){
    if(IsKeyPressed(KEY_A))
{
    player.velocity.x += -slideSpeed;
    sliding = 1; 
}
else if (IsKeyPressed(KEY_D))
{
    player.velocity.x += slideSpeed;
    sliding = 1;
}
else
{
    targetSpeed = 0;
    float speedDif = -player.velocity.x;
    float movement = speedDif*brakingConstant/3;
    applyForce(movement, 0);
}
    }

    
}

if(IsKeyPressed(KEY_S) && grounded){
    player.position.y += (player.size.y - crouchHeight);
    player.size.y = crouchHeight;
}
if(IsKeyDown(KEY_S)){

    if(grounded){
        crouching = 1;
        player.size.y = crouchHeight;
    }

}

if(IsKeyUp(KEY_S) && crouching){
    crouching = 0;
    sliding = 0;
    player.position.y += -(playerHeight - crouchHeight);
    player.size.y = playerHeight;
}

//a = f/m
player.acc.y = (gravity*gravityModifier) + (player.force.y/player.mass);
player.acc.x = player.force.x/player.mass;


                    }





void RunLogic() {


//crouching logic


//check timers
/*
for(const auto& t : vTimers){
    if(GetTime() >= t.second){
        *t.first = 1;
    }
}
*/

//the commented code below was for testing my raycasting implementation, 
//it draws a line from 200, 200 to your cursor and draws a circle where the line intersects the first rectangle in the vRects vector.
/*
Vector2 ray_point = {200.0f, 200.0f};
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
vRects[0].velocity.x += vRects[0].acc.x * GetFrameTime();
vRects[0].velocity.y += vRects[0].acc.y * GetFrameTime();

//debug player
DrawText(TextFormat("X = %f, Y = %f, \n VelX = %f, VelY = %f, \n grounded = %i, crouched = %i, jumping = %i sliding = %i \n, gravMod = %f FPS = %i, width = %f, height = %f, \n brakingConstant = %f, mouseX = %f, mouseY = %f", vRects[0].position.x, vRects[0].position.y, vRects[0].velocity.x, vRects[0].velocity.y, grounded, crouching, jumping, sliding, gravityModifier, GetFPS(), player.size.x, player.size.y, brakingConstant, GetScreenToWorld2D(GetMousePosition(), currentCam).x,GetScreenToWorld2D(GetMousePosition(), currentCam).y ), 10, 10, 20, WHITE);

std::vector<collision> z;
wallslidingRight = 0;
wallslidingLeft = 0;
grounded = 0;

for(int i = 1; i < int(vRects.size()); i++)
{ 
    ray RectRay = DynamicRectVSRect(vRects[0], vRects[i]);
    if(RectRay.collided && RectRay.rayCheck <= 1.0f){
        z.push_back({i, RectRay.rayCheck, RectRay.type});
    }

    
}


//This should theoretically sort the collisions by shortest to longest, then resolve the shortest collision. If i screwed up then please tell me!
std::sort(z.begin(), z.end(), [](const collision& a, const collision& b)
{
    return a.second < b.second;
});

for (auto j : z)
if (DynamicRectVSRect(vRects[0], vRects[j.first]).collided)
{
    ray RectRay = DynamicRectVSRect(vRects[0], vRects[j.first]);
    //grounded detection logic
    if(RectRay.collided && RectRay.rayCheck <= 1 && RectRay.contact_normal.y == -1){
        grounded = 1;
    }
    else{
        grounded = 0;
    }
    
    //wallslide detection logic
    if(RectRay.collided && RectRay.rayCheck <= 1 && RectRay.contact_normal.x == -1){
        wallslidingRight = 1;
    }
    else{
        wallslidingRight = 0;
    }

    if(RectRay.collided && RectRay.rayCheck <= 1 && RectRay.contact_normal.x == 1){
        wallslidingLeft = 1;
    }
    else{
        wallslidingLeft = 0;
    }

    
    //The collision is resolved by truncating the velocity to the point where the moving rectangle can never intersect with the static rectangle
    //I also added a one-pixel buffer around the moving rectangle, as there were some issues with the origin of the raycast being from inside the static rectangle when the pixel buffer was removed.

//Checks the type of rectangle that was collided with. If it's a wall, resolve collision. If it's a spike, kill the player.
if(RectRay.type == 1){
vRects[0].velocity = Vector2Add(Vector2Add(vRects[0].velocity, Vector2{RectRay.contact_normal.x, RectRay.contact_normal.y}), Vector2Multiply(RectRay.contact_normal, Vector2Scale((Vector2){fabsf(vRects[0].velocity.x), fabsf(vRects[0].velocity.y)}, (1-RectRay.rayCheck))));
}
else if(RectRay.type == 2){
playerDeath();
}

}

if(jumping && sliding){
    brakingConstant = 0;
}
else {
    brakingConstant = 30;
}

if(grounded){
groundedCoyoteTimer = GetTime() + groundedCoyoteWindow;
}

if(wallslidingLeft){
wallslideLeftCoyoteTimer = GetTime() + wallslideCoyoteWindow;
}

if(wallslidingRight){
wallslideRightCoyoteTimer = GetTime() + wallslideCoyoteWindow;
}
//caps the player's downwards vertical speed when wallsliding
if((wallslidingLeft || wallslidingRight) && player.velocity.y > 100){
    player.velocity.y = 100;
}

//if the player's velocity is too slow, they won't be counted as sliding anymore
if(std::abs(player.velocity.x) < 200 && sliding){
    sliding = 0;
}

if((std::abs(player.velocity.x) < 1)){
    player.velocity.x = 0;
}
//change the moving rectangle's position by its velocity modulated by deltaTime

vRects[0].position.x += vRects[0].velocity.x * GetFrameTime();
vRects[0].position.y += vRects[0].velocity.y * GetFrameTime();
}


Vector2 rectangleOffset;

void DrawGame(){

//automatically draws each rectangle in the vRects vector, plus a one pixel expansion to make up for the one-pixel buffer i added to the player.
//if there is a more elegant way for this to work, please tell me.
for(const auto& r : vRects){
    Color RectColor;
    rectangleOffset = Vector2 {0,0};

if(r.type == 0) {
    RectColor = YELLOW;
    rectangleOffset = Vector2 {1, 1};
    }
if(r.type == 1) RectColor = WHITE;
if(r.type == 2) RectColor = RED;
    DrawRectangle(r.position.x, r.position.y, r.size.x +rectangleOffset.x, r.size.y+rectangleOffset.y, RectColor);

}


//unused code for graphics, may or may not use later
/*Rectangle source = Rectangle {0, 0, 26, 19};
Rectangle dest = Rectangle {player.position.x-8, player.position.y, source.width*2, source.height*2};

DrawTexturePro(playerSprite, source, dest, Vector2 {0,0}, 0, WHITE);*/

}

void DrawMenus() {

}


int main()
{

    const int screenWidth = 1280;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "2d Collision Prototype");
    SetTargetFPS(60);
    SetupGame();
    saveLevel();
    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(BLACK);
        GetInput();
        RunLogic();
        DrawMenus();
        MoveCamera();
        DrawGame();
        EndMode2D();
        EndDrawing();

    }

    CloseWindow();
    return 0;
}