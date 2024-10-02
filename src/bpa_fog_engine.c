/*******************************************************************************************
*
*   project_name
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
//#include "rcamera.h"  //https://stackoverflow.com/questions/72161657/how-can-i-change-camera3d-movement-speed-in-raylib
//TODO: Implement this: https://www.raylib.com/examples/core/loader.html?name=core_smooth_pixelperfect

//----------------------------------------------------------------------------------
//CAM STUFF
#define RCAMERA_IMPLEMENTATION
//LIGHTING STUFF
#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif
//----------------------------------------------------------------------------------



#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
Camera camera = { 0 };
Vector3 one = {1.0f,1.0f,1.0f };
Vector3 half = {0.5f,0.5f,0.5f};
Vector3 cubePosition = { 0 };
Vector3 cubeScale = { 0.5,0.5f,0.5f };
Vector3 playerPosition ={0.0f,0.5f,0.0f};
Vector3 playerDirectionVector ={1.0f,0.0f,0.0f};
// Define an array of Vector3
    Vector3 bulletLocations[3] = {
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 1.0f },
        { -1.0f, 0.0f, -1.0f }
    };
    Vector3 bulletMoveVectors[3] = {
        {1.0f, 0.0f, 0.0f  },
        {1.0f, 0.0f, 0.0f} ,
        {1.0f, 0.0f, 0.0f }
    };
    float bulletLifetime[3] =
    {
      0.2f,0.5f,0.8f  
    };
    
float floorPlaneYPos=0.5f;
float gravity = 15.2f;
float jumpStrength = 7.2f;
float jumpYPos=0.0f;
float globalTimer=0.0f;
float lastDelta=0.0f;

//------------------------------------------------------

Light lights[MAX_LIGHTS] = { 0 };
//Models
Model model;
Shader shader;
void InitModel() {
    model = LoadModel("resources/models/AnotherDummy.obj");
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE; // Set diffuse color to white
    //Texture2D texture = LoadTexture("resources/models/testTexture.png"); // Load model texture
    //model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;            // Set map diffuse texture
}
void DrawModelsInScene() {
   //SIMPLE//  DrawModel(model, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
   Vector3 pos={0.0f,0.0f,0.0f};
   pos=playerPosition;
   Vector3 rotAxis={0.0f,1.0f,0.0f};
   float angle=globalTimer*200.0f;
   DrawModelEx(model, pos, rotAxis, angle, half, WHITE); 
}
//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);          // Update and draw one frame

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main()
{
    

      
      
    //cool examples: https://www.raylib.com/examples/core/loader.html?name=core_smooth_pixelperfect
    //https://ziggit.dev/t/how-to-get-the-screen-buffer-from-the-raylib-image-to-be-able-to-draw-pixels-and-push-it-to-the-window/5612/2
    
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800*2;
    const int screenHeight = 450*2;
    
    InitWindow(screenWidth, screenHeight, "F E");

    camera.position = (Vector3){ 3.0f, 3.0f, 2.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    //SETUP MODELS
    //--------------------------------------------------------------------------------------
    InitModel();

    //LIGHTS
    shader = LoadShader("resources/shaders/glsl330/lighting.vs", "resources/shaders/glsl330/lighting.fs");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[]){ 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);
    //--

lights[0] = CreateLight(LIGHT_POINT, (Vector3){ -2, 1, -2 }, Vector3Zero(), YELLOW, shader);
lights[1] = CreateLight(LIGHT_POINT, (Vector3){ 2, 1, 2 }, Vector3Zero(), RED, shader);
lights[2] = CreateLight(LIGHT_POINT, (Vector3){ -2, 1, 2 }, Vector3Zero(), GREEN, shader);
lights[3] = CreateLight(LIGHT_POINT, (Vector3){ 2, 1, -2 }, Vector3Zero(), BLUE, shader);
    
    //--------------------------------------------------------------------------------------
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif
    //Unload models
    UnloadModel(model);   
    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
static void PlayerStuff(void)
{
    //MISC
    float delta = GetFrameTime(); 
   
    float speed = 5.6f;
    bool  jump  = false;
    globalTimer+=delta;
    //CAM STUFF
    
    
    
    Vector3 forward = Vector3Subtract(camera.target, camera.position);
    forward.y = 0.0f; // Project forward vector onto the horizontal plane
    forward = Vector3Normalize(forward); // Normalize the forward vector
    Vector3 up = { 0.0f, 1.0f, 0.0f }; // Assuming Y is up
    Vector3 right = Vector3CrossProduct(forward, up);
    
// Load basic lighting shader
   /* Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
                               TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));
     */                          

    //CONTROLS
    float horizontalAxis = 0.0f;
    if (IsKeyDown(KEY_D)){horizontalAxis = 1.0f;}
    if (IsKeyDown(KEY_A)){horizontalAxis = -1.0f;}
    float verticalAxis = 0.0f;
    if (IsKeyDown(KEY_W)){verticalAxis = 1.0f;}
    if (IsKeyDown(KEY_S)){verticalAxis = -1.0f;}
    if (IsKeyDown(KEY_SPACE))
    {
        if(playerPosition.y<=floorPlaneYPos)
        {
            jump=true;
        }
        

    }
    //---
    //normalize input (ONLY USE THIS ONCE YOU IMPLEMENT A CUSTOM CAMERA!)
    /*Vector2 normalizeInputVector = {horizontalAxis,verticalAxis};
    normalizeInputVector = Vector2Normalize(normalizeInputVector);
    horizontalAxis=normalizeInputVector.x;
    verticalAxis=normalizeInputVector.y;*/
    //---
    
    //MOVEMENT
    Vector3 moveVector = {0.0f,0.0f,0.0f};
 
    
    Vector3 a = Vector3Scale (right,horizontalAxis*speed*delta);
    Vector3 b = Vector3Scale (forward,verticalAxis*speed*delta);
    
    moveVector =  Vector3Add(a,b);
    
  
     
    //Jump and Gravity
    jumpYPos-=gravity*delta;
    if(jump)
    {
        jumpYPos = jumpStrength;
        
    }
     playerPosition.y = Lerp(playerPosition.y,jumpYPos,speed*delta);
    //FLOOR COLLISION
    if(playerPosition.y <= floorPlaneYPos)
    {
        
        playerPosition.y = floorPlaneYPos;
    }
    //APPLY MOVE
    playerPosition = Vector3Add(playerPosition,moveVector);
    
      //Capture move direction for the gun later.
    if(Vector3Length(moveVector)>0)
    {
        playerDirectionVector=Vector3Scale(moveVector,100.0f);
    playerDirectionVector=Vector3Normalize(playerDirectionVector);
    playerDirectionVector.y=0.0f;
    }
    

    
    //DRAW PLAYER AT POS
    float sinVal = sin(globalTimer*12)*0.02f;
    float cosVal = cos(globalTimer*12)*0.02f;
    DrawCube(playerPosition,cubeScale.x+sinVal,(cubeScale.y*2)+cosVal,cubeScale.z+sinVal, RED);
  
}
void BulletLogic()
{
    float deltaTime=GetFrameTime();
    float speed = 10.6f;
    float maxLife = 0.9f;
    for(int i=0; i< 3;++i)
    {
        //MOVE ACTIVE BULLETS
        Vector3 modifiedMoveVector = {bulletMoveVectors[i].x,bulletMoveVectors[i].y,bulletMoveVectors[i].z};
        modifiedMoveVector = Vector3Normalize(modifiedMoveVector);
        modifiedMoveVector = Vector3Scale (bulletMoveVectors[i],speed*2*deltaTime);
       
         
        bulletLocations[i] =  Vector3Add(bulletLocations[i],modifiedMoveVector);
        
         float sinVal = sin(globalTimer*24)*0.01f;
        
         DrawSphere(bulletLocations[i],0.1f+sinVal, BLUE);
         // DrawLine3D(bulletLocations[i],playerPosition,RED);
          DrawCapsule(bulletLocations[i],playerPosition,0.02f,8,8,GREEN);
        //LIFETIME COUNTER
        bulletLifetime[i]+=2*deltaTime;
        //TODO: REPOOL
        if(bulletLifetime[i]>=maxLife)
        {
            bulletLifetime[i]=0.0f;
            bulletLocations[i] =  playerPosition;
            bulletMoveVectors[i]=playerDirectionVector;
        }
    }
}
void WeirdFX()
{
    //LINES AND STUFF
    Vector3 startPos = {0.0f,0.5f,0.0f};
     Vector3 startPos2 = {0.0f,1.5f,0.0f};
     DrawCapsule(startPos,playerPosition,0.02f,8,8,BLUE);
     DrawCapsule(startPos2,playerPosition,0.02f,8,8,BLUE);
     
     
     //LIGHTING
     for (int i = 0; i < MAX_LIGHTS; i++) {
        if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
        else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
    }
    
    
}
bool CollisionCheck(Vector3 playerPosition,Vector3 enemyplayerPosition, Vector3 playerSize, Vector3 enemyBoxSize)
{
    bool collision=false;
        if (CheckCollisionBoxes(
            (BoundingBox){(Vector3){ playerPosition.x - playerSize.x/2,
                                     playerPosition.y - playerSize.y/2,
                                     playerPosition.z - playerSize.z/2 },
                          (Vector3){ playerPosition.x + playerSize.x/2,
                                     playerPosition.y + playerSize.y/2,
                                     playerPosition.z + playerSize.z/2 }},
            (BoundingBox){(Vector3){ enemyplayerPosition.x - enemyBoxSize.x/2,
                                     enemyplayerPosition.y - enemyBoxSize.y/2,
                                     enemyplayerPosition.z - enemyBoxSize.z/2 },
                          (Vector3){ enemyplayerPosition.x + enemyBoxSize.x/2,
                                     enemyplayerPosition.y + enemyBoxSize.y/2,
                                     enemyplayerPosition.z + enemyBoxSize.z/2 }})) collision = true;
     return collision;
}
void GameUpdate()
{
    
        PlayerStuff();
        BulletLogic();
        DrawModelsInScene();
        WeirdFX();
        
        
        DrawGrid(20, 1.0f);
}
void GameUpdate2D()
{
    DrawText(   TextFormat("Stats:  FPS:%i",GetFPS())  , 10, 20, 20, DARKGRAY);
    DrawText(  TextFormat("LOC: \nX:%.2f \nY:%1.2f \nZ:%0.2f",playerPosition.x,playerPosition.y,playerPosition.z)  , 10, 65, 20, DARKGRAY);
    DrawText(  TextFormat("DIR: \nX:%.2f \nY:%.2f \nZ:%.2f",playerDirectionVector.x,playerDirectionVector.y,playerDirectionVector.z)  , 80, 65, 20, DARKGRAY);
}
// Update and draw game frame
static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    //UpdateCamera(&camera, CAMERA_THIRD_PERSON);
    UpdateCamera(&camera, CAMERA_THIRD_PERSON);
    
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    for (int i = 0; i < MAX_LIGHTS; i++) UpdateLightValues(shader, lights[i]);
    
    BeginDrawing();
    
        
    //BeginShaderMode(shader); //remove me until lighting works...


    ClearBackground(LIGHTGRAY);

    BeginMode3D(camera);
    //GAME STUFF HERE:
    GameUpdate();

    
      
   // EndShaderMode();//remove me until lighting works...
    
    
    EndMode3D();
    //2D Draw
    GameUpdate2D();

      
        
       

    EndDrawing();
    //----------------------------------------------------------------------------------
}
