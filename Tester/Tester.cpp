// Tester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../VEngineNative/Camera.h";
#include "../VEngineNative/Object3dInfo.h";
#include "../VEngineNative/Media.h";
#include "../VEngineNative/World.h";
#include "../VEngineNative/Scene.h";
#include "../VEngineNative/Material.h";
#include "../VEngineNative/Mesh3d.h";
#include "../VEngineNative/Light.h";
#include "../VEngineNative/SquirrelVM.h";
#include "../VEngineNative/GLSLVM.h";
#include "../VEngineNative/SimpleParser.h";
#include "../VEngineNative/imgui/imgui.h";

#include "Car.h";

Mesh3d * loadRawMesh(string file) {
    Material *mat = new Material();

    unsigned char* bytes;
    int bytescount = Media::readBinary(file, &bytes);
    GLfloat * floats = (GLfloat*)bytes;
    int floatsCount = bytescount / 4;
    vector<GLfloat> flo(floats, floats + floatsCount);

    Object3dInfo *o3i = new Object3dInfo(flo);

    return Mesh3d::create(o3i, mat);
}

int main()
{

    Media::loadFileMap("../../media");
    Media::loadFileMap("../../shaders");
    Game *game = new Game(1920, 1020);
    game->start();
    volatile bool ready = false;
    game->invoke([&ready]() {
        ready = true;
    });
    while (!ready);

    SimpleParser* par = new SimpleParser();
    auto axs = par->tokenize("diffuseColor = gradient("
        "    0.0, texture0,"
        "    0.4f, texture1,"
        "    8, 123556564, \"loader\" + texture2,"
        "    pow(texture3, 2.0);"
        ");"
    );

    // lets test this volatile piece of shit


    for (int i = 0; i < axs.size(); i++) {
        if (axs[i].type == TOKEN_NAME) printf("TOKEN_NAME %s\n", axs[i].strdata);
        if (axs[i].type == TOKEN_ASSIGNMENT) printf("TOKEN_ASSIGNMENT %s\n", axs[i].strdata);
        if (axs[i].type == TOKEN_OPERATOR_ADD) printf("TOKEN_OPERATOR_ADD %s\n", axs[i].strdata);
        if (axs[i].type == TOKEN_OPERATOR_SUB) printf("TOKEN_OPERATOR_SUB %s\n", axs[i].strdata);
        if (axs[i].type == TOKEN_OPERATOR_MUL) printf("TOKEN_OPERATOR_MUL %s\n", axs[i].strdata);
        if (axs[i].type == TOKEN_OPERATOR_DIV) printf("TOKEN_OPERATOR_DIV %s\n", axs[i].strdata);
        if (axs[i].type == TOKEN_OPERATOR_POW) printf("TOKEN_OPERATOR_POW %s\n", axs[i].strdata);
        if (axs[i].type == TOKEN_STRING) printf("TOKEN_STRING %s\n", axs[i].strdata);
        if (axs[i].type == TOKEN_SEMICOLON) printf("TOKEN_SEMICOLON %s\n",   axs[i].strdata);
        if (axs[i].type == TOKEN_BRACE_OPEN) printf("TOKEN_BRACE_OPEN %s\n", axs[i].strdata);
        if (axs[i].type == TOKEN_BRACE_CLOSE) printf("TOKEN_BRACE_CLOSE %s\n", axs[i].strdata);
        if (axs[i].type == TOKEN_CURLY_OPEN) printf("TOKEN_CURLY_OPEN %s\n", axs[i].strdata);
        if (axs[i].type == TOKEN_CURLY_CLOSE) printf("TOKEN_CURLY_CLOSE %s\n", axs[i].strdata);
        if (axs[i].type == TOKEN_COMMA) printf("TOKEN_COMMA %s\n", axs[i].strdata);
    }

    Camera *cam = new Camera();
    float fov = 95.0f;
    float fovnew = 95.0f;
    cam->createProjectionPerspective((95.0f), (float)game->width / (float)game->height, 0.01f, 1000);
    game->onWindowResize->add([&](int zero) {
        cam->createProjectionPerspective((95.0f), (float)game->width / (float)game->height, 0.01f, 1000);
    });
    cam->transformation->translate(glm::vec3(16, 16, 16));
    glm::quat rot = glm::quat_cast(glm::lookAt(cam->transformation->position, glm::vec3(0), glm::vec3(0, 1, 0)));
    cam->transformation->setOrientation(rot);
    game->world->mainDisplayCamera = cam;

    // mesh loading

  //  SquirrelVM* sq = new SquirrelVM();
  //  sq->compileFile(Media::getPath("squireeltest.txt"));
  //  sq->callProcedureVoid("testme");

    game->world->scene = game->asset->loadSceneFile("sp.scene");
    //game->world->scene->getMeshes()[0]->getInstance(0)->transformation->translate(glm::vec3(0, -62.5f, 0));
   // for (int i = 0; i < game->world->scene->getMeshes().size(); i++) {
     //   game->world->scene->getMeshes()[i]->getInstance(0)->transformation->scale(glm::vec3(100.0f));
  //  }
   //game->world->scene->getMeshes()[0]->getInstance(0)->transformation->rotate(glm::angleAxis(deg2rad(73.75f), glm::vec3(-0.006f, -0.005f, 1.0f)));
 //  game->world->scene->addMesh(game->asset->loadMeshFile("treeground.mesh3d"));

    auto car = new Car();
    auto t = game->asset->loadMeshFile("2dplane.mesh3d");
    t->alwaysUpdateBuffer = true;
    game->world->scene->addMesh(t);
    /*
    for (int x = 0; x < 1; x++) {
        for (int y = 0; y < 50; y++) {
            t->addInstance(new Mesh3dInstance(new TransformationManager(glm::vec3(x, y + 20.0, 0.0))));
        }
    }

    */
    game->invoke([&]() {
        auto phys = Game::instance->world->physics;
        auto groundpb = phys->createBody(0.0f, new TransformationManager(glm::vec3(0.0, -0.0, 0.0)), new btBoxShape(btVector3(1111.0, 5.0, 1110.0)));
        //groundpb->body->applyTorque(btVector3(1000, 1000, 1000));
        groundpb->body->setFriction(1010);
        groundpb->enable();
        /*
        PhysicalBody* last = nullptr;
        for (int i = 0; i < t->getInstances().size(); i++) {
            auto cube = phys->createBody(1.0f, t->getInstance(i)->transformation, new btBoxShape(btVector3(1.0, 1.0, 1.0)));
            cube->enable();
            if (last != nullptr) {
                auto rawconstraint = new btConeTwistConstraint(*(last->body), *(cube->body), btTransform(btQuaternion(), btVector3(0.0, 1.0, 0.0)), btTransform(btQuaternion(), btVector3(0.0, -1.0, 0.0)));
                rawconstraint->setBreakingImpulseThreshold(9999999.0);
                auto con = new PhysicalConstraint(rawconstraint, last, cube);
                con->enable();
            }
            last = cube;
        }*/
    });
    auto flagbase = game->asset->loadMeshFile("flagbase.mesh3d");
    game->world->scene->addMesh(flagbase);
    GLSLVM* vm = new GLSLVM();
    game->invoke([&]() {/*
        vm->resizeBufferFloat(1);

        int data[1000];
        auto programAssembly = vector<int>{
            MASM_DIRECTIVE_MOV_INVOCATION_ID, 7, // move invocation id into int reg 7

            MASM_DIRECTIVE_LOAD_INT, 2, 77, // move value at addres 77 which is 100 to int reg 2
            MASM_DIRECTIVE_ADD_INT, 7, 2, // int reg 7 += int reg 2 (inv id + 100)
            MASM_DIRECTIVE_LOAD_BY_POINTER_INT, 0, 7, // load by pointer in int reg 7 which is 100 + inv id < its OK
            MASM_DIRECTIVE_LOAD_INT, 1, 78, // load value text length into int reg 1
            MASM_DIRECTIVE_ADD_INT, 7, 1, // int reg 8 += int reg 1 which is text length
            MASM_DIRECTIVE_STORE_BY_POINTER_INT, 7, 0, // store there 

        };
        for (int i = 0; i < programAssembly.size(); i++)data[i] = programAssembly[i];
        vm->resizeBufferInt(1000);

        vm->bufferSubDataInt(0, 1000, &data[0]);
        vm->run(teststring.size());
        int result[100];
        vm->readSubDataInt(100, teststring.size() * 2, &result[0]);
        for (int i = 0; i < teststring.size() * 2; i++)printf("%c", (char)result[i]);
        // EXPECTING 2.0f
        printf("RESU:LT I");// EXPECTING 2.0f*/
    });


    bool isOpened = true;
    bool isOpened2 = true;

    unsigned char* pixels;
    int width, height;
    //io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
    bool imugiinit = false;
    bool displayimgui = true;
    game->onRenderUIFrame->add([&](int zero) {
        if (displayimgui) {
            if (!imugiinit) {
                //    ImGuiIO& io = ImGui::GetIO();
                    // io.Fonts->ClearFonts();
                    // io.Fonts->AddFontFromFileTTF(Media::getPath("segoeui.ttf").c_str(), 10);
                imugiinit = true;
            }
            if (fov != fovnew) {
                cam->createProjectionPerspective((fovnew), (float)game->width / (float)game->height, 0.01f, 1000);
                fov = fovnew;
            }
            static float f = 0.0f;/*
            float a1 = Game::instance->renderer->cloudsThresholdLow;
            float a2 = Game::instance->renderer->cloudsThresholdHigh;
            if (Game::instance->renderer->cloudsThresholdHigh < Game::instance->renderer->cloudsThresholdLow) {
                a1 = Game::instance->renderer->cloudsThresholdHigh;
                a2 = Game::instance->renderer->cloudsThresholdHigh;
            }
            if (Game::instance->renderer->cloudsThresholdLow > Game::instance->renderer->cloudsThresholdHigh) {
                a1 = Game::instance->renderer->cloudsThresholdLow;
                a2 = Game::instance->renderer->cloudsThresholdLow;
            }
            Game::instance->renderer->cloudsThresholdLow = a1;
            Game::instance->renderer->cloudsThresholdHigh = a2;*/

            auto a1 = Game::instance->renderer->cloudsFloor;
            auto a2 = Game::instance->renderer->cloudsCeil;
            if (Game::instance->renderer->cloudsCeil < Game::instance->renderer->cloudsFloor) {
                a1 = Game::instance->renderer->cloudsCeil;
                a2 = Game::instance->renderer->cloudsCeil;
            }
            if (Game::instance->renderer->cloudsFloor > Game::instance->renderer->cloudsCeil) {
                a1 = Game::instance->renderer->cloudsFloor;
                a2 = Game::instance->renderer->cloudsFloor;
            }
            Game::instance->renderer->cloudsFloor = a1;
            Game::instance->renderer->cloudsCeil = a2;

            ImGui::Begin("README", &isOpened2, 0);
            ImGui::Text("You can mimize this message by double clicking on its top bar");
            ImGui::Text("Thank you for trying out this demo!");
            ImGui::Text("Source code can be found here https://github.com/achlubek/venginenative");
            ImGui::Text("Window size changing works but can sometimes break GUI");
            ImGui::Separator();
            ImGui::Text("Controls:");
            ImGui::Text("* Press TAB to capture mouse and look around");
            ImGui::Text("* Use WASD keys to move around");
            ImGui::Text("* Use CTRL, SHIFT, ALT to control movement speed");
            ImGui::Separator();
            ImGui::Text("Settings meaning:");
            ImGui::Text("* FOV - Main camera fov");
            ImGui::Text("* CloudsHeightStart - How high clouds should start (in kilometers)");
            ImGui::Text("* CloudsHeightEnd - How high clouds should end (in kilometers)");
            ImGui::Text("* CloudsThresholdLow - Lower this to get more clouds");
            ImGui::Text("* CloudsThresholdHigh - Put it closed to CloudsThresholdLow to get more sharp clouds");
            ImGui::Text("* CloudsDensity - Cloud density, used in shadows");
            ImGui::Text("* CloudsIntegration - Higher this to make clouds less noisy, lower this if you change something");
            ImGui::Text("* CloudsOffset - Clouds offset over the sky, you can use Y coord to change clouds seed somehow");
            ImGui::Text("* SunDirection - Sky direction, (0, 1, 0) is UP");
            ImGui::Text("* MieScattering - Air pollution, close to zero is clear, max Pekin, zero is broken");
            ImGui::Text("* GodRaysStrength - How strong should god rays be");
            ImGui::Text("* WaterWavesHeight - Water terrain mesh horizontal scale");
            ImGui::Text("* WaterScale - Water terrain mesh vertical scale");
            ImGui::End();

            ImGui::Begin("Clouds", &isOpened, 0);
            ImGui::Text("%.3f ms/frame %.1f FPS", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            //    ImGui::Text("Terrain roughness:");
           //     ImGui::SliderFloat("roughness", &t->getLodLevel(0)->material->roughness, 0.0f, 1.0f);
           //     ImGui::Text("Terrain metalness:");
           //     ImGui::SliderFloat("metalness", &t->getLodLevel(0)->material->metalness, 0.0f, 1.0f);
            ImGui::SliderFloat("FOV", &fovnew, 5.0f, 175.0f);
            ImGui::SliderFloat("Exposure", &Game::instance->renderer->exposure, 0.01f, 10.0f);
            ImGui::SliderFloat("Contrast", &Game::instance->renderer->contrast, 0.01f, 10.0f);
            ImGui::SliderFloat("CloudsHeightStart", &Game::instance->renderer->cloudsFloor, 100.0f, 30000.0f);
            ImGui::SliderFloat("CloudsHeightEnd", &Game::instance->renderer->cloudsCeil, 100.0f, 30000.0f);
            ImGui::SliderFloat("CloudsThreshold", &Game::instance->renderer->cloudsThresholdLow, 0.0f, 1.0f);
            ImGui::SliderFloat("CloudsThresholdHigh", &Game::instance->renderer->cloudsThresholdHigh, 0.0f, 1.0f);
            //ImGui::SliderFloat("CloudsAtmosphereShaftsMultiplier", &Game::instance->renderer->cloudsAtmosphereShaftsMultiplier, 0.0f, 10.0f);
            //ImGui::SliderFloat("CloudsWindSpeed", &Game::instance->renderer->cloudsWindSpeed, 0.0f, 10.0f);
            ImGui::SliderFloat("CloudsDensity", &Game::instance->renderer->cloudsDensityScale, 0.0f, 5.0f);
            ImGui::SliderFloat("CloudsIntegration", &Game::instance->renderer->cloudsIntegrate, 0.3f, 0.999f);
            ImGui::SliderFloat3("CloudsOffset", (float*)&Game::instance->renderer->cloudsOffset, -1000.0f, 1000.0f);
            ImGui::Separator();
            ImGui::SliderFloat("DayElapsed", (float*)&Game::instance->renderer->dayElapsed, 0.0f, 1.0f);
            ImGui::SliderFloat("YearElapsed", (float*)&Game::instance->renderer->yearElapsed, 0.0f, 1.0f);
            ImGui::SliderFloat("Equator/Pole", (float*)&Game::instance->renderer->equatorPoleMix, 0.0f, 1.0f);

            ImGui::SliderFloat("MieScattering", (float*)&Game::instance->renderer->mieScattCoefficent, 0.0f, 20.0f);
            ImGui::SliderFloat("GodRaysStrength", &Game::instance->renderer->noiseOctave1, 0.01f, 10.0f);
            ImGui::Separator();
            ImGui::SliderFloat("WaterWavesHeight", &Game::instance->renderer->waterWavesScale, 0.0f, 10.0f);
            ImGui::SliderFloat("WaterSpeed", &Game::instance->renderer->waterSpeed, 0.0f, 7.0f);
            ImGui::SliderFloat2("WaterScale", &Game::instance->renderer->waterScale.x, 0.0f, 10.0f);
            //        ImGui::SliderFloat2("MeshRoughness", &t->getLodLevel(0)->material->roughness, 0.0f, 1.0f);
                    //ImGui::SliderFloat("CloudsDensityThresholdLow", &Game::instance->renderer->cloudsDensityThresholdLow, 0.0f, 1.0f);
                   // ImGui::SliderFloat("CloudsDensityThresholdHigh", &Game::instance->renderer->cloudsDensityThresholdHigh, 0.0f, 1.0f);
                    //ImGui::SliderFloat("AtmosphereScale", &Game::instance->renderer->atmosphereScale, 0.0f, 1000.0f);
                   // ImGui::SliderFloat("Noise2", &Game::instance->renderer->noiseOctave2, 0.01f, 10.0f);
                   // ImGui::SliderFloat("Noise3", &Game::instance->renderer->noiseOctave3, 0.01f, 10.0f);
                  //  ImGui::SliderFloat("Noise4", &Game::instance->renderer->noiseOctave4, 0.01f, 10.0f);
                  //  ImGui::SliderFloat("Noise5", &Game::instance->renderer->noiseOctave5, 0.01f, 400.0f);
                  //  ImGui::SliderFloat("Noise6", &Game::instance->renderer->noiseOctave6, 0.01f, 10.0f);
                  //  ImGui::SliderFloat("Noise7", &Game::instance->renderer->noiseOctave7, 0.01f, 10.0f);
                  //  ImGui::SliderFloat("Noise8", &Game::instance->renderer->noiseOctave8, 0.01f, 10.0f);
            ImGui::SliderFloat("LensBlurAmount", &Game::instance->renderer->lensBlurSize, 0.01f, 10.0f);
            ImGui::SliderFloat("FocalLength", &cam->focalLength, 0.01f, 10.0f);
            //ImGui::SliderFloat("Noise8", &Game::instance->renderer->noiseOctave8, 0.01f, 10.0f);
    //        ImGui::SliderFloat("RoughnessTerra", &game->world->scene->getMeshes()[0]->getLodLevel(0)->material->roughness, 0.01f, 10.0f);
            for (auto i = Game::instance->renderer->cloudsShader->shaderVariables.begin(); i != Game::instance->renderer->cloudsShader->shaderVariables.end(); i++) {
                if (i->second->type == SHADER_VARIABLE_TYPE_FLOAT) ImGui::SliderFloat(i->first.c_str(), &i->second->var_float, 0.01f, 10.0f);
                if (i->second->type == SHADER_VARIABLE_TYPE_INT) ImGui::SliderInt(i->first.c_str(), &i->second->var_int, 0, 20);
                if (i->second->type == SHADER_VARIABLE_TYPE_VEC2) ImGui::SliderFloat2(i->first.c_str(), &i->second->var_vec2.x, 0.01f, 10.0f);
                if (i->second->type == SHADER_VARIABLE_TYPE_IVEC2) ImGui::SliderInt2(i->first.c_str(), &i->second->var_ivec2.x, 0.0, 20);
                if (i->second->type == SHADER_VARIABLE_TYPE_VEC3) ImGui::SliderFloat3(i->first.c_str(), &i->second->var_vec3.x, 0.01f, 10.0f);
                if (i->second->type == SHADER_VARIABLE_TYPE_IVEC3) ImGui::SliderInt3(i->first.c_str(), &i->second->var_ivec3.x, 0.0, 20);
                if (i->second->type == SHADER_VARIABLE_TYPE_VEC4) ImGui::SliderFloat4(i->first.c_str(), &i->second->var_vec4.x, 0.01f, 10.0f);
                if (i->second->type == SHADER_VARIABLE_TYPE_IVEC4) ImGui::SliderInt4(i->first.c_str(), &i->second->var_ivec4.x, 0.0, 20);
            }
            ImGui::End();
        }
    });
    /*
    for (int i = 0; i < 11; i++) {
        for (int g = 0; g < 11; g++) {
            float rough = (float)i / 11.0f;
            float met = (float)g / 11.0f;
            Mesh3d* mesh = game->asset->loadMeshFile("icosphere.mesh3d");
            mesh->getLodLevel(0)->material->roughness = rough;
            mesh->getLodLevel(0)->material->metalness = met;
            mesh->getInstance(0)->transformation->translate(glm::vec3(i, 0, g) * 8.0f);
            game->world->scene->addMesh(mesh);
        }
    }
    */
    // Light* light = game->asset->loadLightFile("test.light");
    // light->type = LIGHT_SPOT;
    // light->angle = 78;
     //light->cutOffDistance = 90;
    // game->world->scene->addLight(light);

    bool cameraFollowCar = false;
    bool cursorFree = false;
    game->onKeyPress->add([&game, &cursorFree, &cam, &displayimgui, &cameraFollowCar](int key) {
        if (key == GLFW_KEY_PAUSE) {
            game->shaders->materialShader->recompile();
            game->shaders->depthOnlyShader->recompile();
            game->renderer->recompileShaders();
        }
        if (key == GLFW_KEY_O) {
            // displayimgui = !displayimgui;
        }
        if (key == GLFW_KEY_C) {
            cameraFollowCar = !cameraFollowCar;
        }
        if (key == GLFW_KEY_0) {
            game->renderer->useAmbientOcclusion = !game->renderer->useAmbientOcclusion;
        }
        if (key == GLFW_KEY_F2) {
            Light* ca = game->asset->loadLightFile("candle.light");
            ca->transformation->position = cam->transformation->position;
            ca->transformation->orientation = cam->transformation->orientation;
            game->world->scene->addLight(ca);
        }
        if (key == GLFW_KEY_F3) {
            Light* ca = game->asset->loadLightFile("corrector.light");
            ca->transformation->position = cam->transformation->position;
            ca->transformation->orientation = cam->transformation->orientation;
            game->world->scene->addLight(ca);
        }
        if (key == GLFW_KEY_TAB) {
            if (!cursorFree) {
                cursorFree = true;
                game->setCursorMode(GLFW_CURSOR_NORMAL);
            }
            else {
                cursorFree = false;
                game->setCursorMode(GLFW_CURSOR_DISABLED);
            }
        }
    });

    float yaw = 0.0f, pitch = 0.0f;
    double lastcx = 0.0f, lastcy = 0.0f;
    bool intializedCameraSystem = false;

    game->setCursorMode(GLFW_CURSOR_DISABLED);

    glm::vec3 newpos = cam->transformation->position;

    float speed = 0.0f;
    glm::vec3 dir = glm::vec3(0);
    glm::vec3 backvectorlast = glm::vec3(0);
    glm::quat backquat = glm::quat();
    game->onRenderFrame->add([&](int i) {
        //  printf("%d\n", game->renderer->pickingResult);
         // game->renderer->pick(cam, glm::vec2(0.5));
         // t->needBufferUpdate = true;

        int cnt = flagbase->getLodLevel(0)->info3d->vbo.size();
        for (int i = 0; i < cnt; i+=12) {
            glm::vec3 vector;
            vector.x = flagbase->getLodLevel(0)->info3d->vbo[i];
            vector.y = flagbase->getLodLevel(0)->info3d->vbo[i+1];
            vector.z = flagbase->getLodLevel(0)->info3d->vbo[i+2];

            vector.z = 0.2 * glm::sin(game->time * 3.0f + vector.x * 5.4 + vector.y * 0.4);

            flagbase->getLodLevel(0)->info3d->vbo[i + 2] = vector.z;
        }
        flagbase->getLodLevel(0)->info3d->rebufferVbo(flagbase->getLodLevel(0)->info3d->vbo, true);

        if (!cursorFree) {
            float maxspeed = 0.1;
            if (game->getKeyStatus(GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
                maxspeed *= 0.1f;
            }
            if (game->getKeyStatus(GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
                maxspeed *= 10;
            }
            if (game->getKeyStatus(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                maxspeed *= 30.1f;
            }/*
            if (game->getKeyStatus(GLFW_KEY_F1) == GLFW_PRESS) {
                light->transformation->position = cam->transformation->position;
                light->transformation->orientation = cam->transformation->orientation;
            }*/
            glm::vec3 dw = glm::vec3(0);
            float w = 0.0;
            bool move = false;
            // speed = speed * 0.99995;
            if (game->getKeyStatus(GLFW_KEY_W) == GLFW_PRESS) {
                dw += cam->transformation->orientation * glm::vec3(0, 0, -1);
                w += 1.0;
                move = true;
            }
            if (game->getKeyStatus(GLFW_KEY_S) == GLFW_PRESS) {
                dw += cam->transformation->orientation * glm::vec3(0, 0, 1);
                w += 1.0;
                move = true;
            }
            if (game->getKeyStatus(GLFW_KEY_A) == GLFW_PRESS) {
                dw += cam->transformation->orientation * glm::vec3(-1, 0, 0);
                w += 1.0;
                move = true;
            }
            if (game->getKeyStatus(GLFW_KEY_D) == GLFW_PRESS) {
                dw += cam->transformation->orientation * glm::vec3(1, 0, 0);
                w += 1.0;
                move = true;
            }
            if (!move) {
                speed = speed / 1.05;
            }
            else {
                speed = glm::mix(speed, maxspeed, 0.02);
            }
            glm::vec3 a = dw / w;
            //dir = mix(dir, w > 0.0 ? a : dw, 0.02);
            dir = w > 0.0 ? a : dw;
            newpos += length(dir) > 0.0 ? (normalize(dir) * speed) : (glm::vec3(0.0));

            if (game->getKeyStatus(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                game->shouldClose = true;
            }
            glm::dvec2 cursor = game->getCursorPosition();
            if (!intializedCameraSystem) {
                lastcx = cursor.x;
                lastcy = cursor.y;
                intializedCameraSystem = true;
            }
            if (cameraFollowCar) {
                int acnt = 0;
                const float * axes = glfwGetJoystickAxes(0, &acnt);
                if (acnt >= 1) {
                    car->setWheelsAngle(axes[0] * 0.9);
                }
                if (acnt >= 6) {
                    float acc = (axes[5] * 0.5 + 0.5);
                    float brk = (axes[4] * 0.5 + 0.5);
                    car->setAcceleration((acc - brk) * 0.5);
                }
                auto cartrans = car->getTransformation();
                if (cartrans != nullptr) {
                    glm::vec3 backvector = glm::vec3(0.0, 1.0, -4.0);
                    auto trsf = glm::quat();
                    if (acnt >= 4) {
                        trsf = glm::angleAxis(deg2rad(axes[3] * 80.0f), glm::vec3(-1.0, 0.0, 0.0)) * glm::angleAxis(deg2rad(axes[2] * 80.0f), glm::vec3(0.0, -1.0, 0.0));
                        auto trsf2 = glm::angleAxis(deg2rad(axes[3] * 80.0f), glm::vec3(1.0, 0.0, 0.0)) *  glm::angleAxis(deg2rad(axes[2] * 80.0f), glm::vec3(0.0, 1.0, 0.0));
                        backvector = trsf2 *  (backvector);
                    }
                    backvector = glm::mat3_cast(glm::inverse(cartrans->orientation)) * backvector;
                    backquat = glm::slerp(backquat, trsf, 0.03f);
                    trsf = backquat;

                    backvector = backvectorlast * 0.97f + backvector * 0.03f;
                    backvectorlast = backvector;
                    cam->transformation->setPosition(cartrans->position + backvector);
                    cam->transformation->setOrientation(glm::inverse(glm::angleAxis(deg2rad(180.0f), glm::vec3(0.0, 1.0, 0.0)) * backquat * cartrans->orientation));
                }

            }
            else {
                float dx = (float)(lastcx - cursor.x);
                float dy = (float)(lastcy - cursor.y);
                lastcx = cursor.x;
                lastcy = cursor.y;
                yaw += dy * 0.2f;
                pitch += dx * 0.2f;
                if (yaw < -90.0) yaw = -90;
                if (yaw > 90.0) yaw = 90;
                if (pitch < -360.0f) pitch += 360.0f;
                if (pitch > 360.0f) pitch -= 360.0f;
                glm::quat newrot = glm::angleAxis(deg2rad(pitch), glm::vec3(0, 1, 0)) * glm::angleAxis(deg2rad(yaw), glm::vec3(1, 0, 0));
                cam->transformation->setOrientation(newrot);
                cam->transformation->setPosition(newpos);
            }

        }
    });

    while (!game->hasExited) {
    }
    return 0;
}