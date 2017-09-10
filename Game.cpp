//#include <Bullet/btBulletDynamicsCommon.h>

#include "Global.h"
#include "CharacterController.h"

//#undef new

class Game : public Application
{
    URHO3D_OBJECT(Game, Application);

public:
    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    float yaw_ = 0.0f;
    float pitch_ = 0.0f;

    Node* charNode_ = nullptr;

    Game(Context* context) : Application(context)
    {
        SubscribeToEvent(E_BEGINFRAME, URHO3D_HANDLER(Game, ApplyGameState));

        CharacterController::RegisterObject(context_);
    }

    void Setup()
    {
        engineParameters_[EP_WINDOW_TITLE] = GetTypeName();
        //engineParameters_[EP_LOG_NAME] = FILE_SYSTEM->GetAppPreferencesDir("urho3d", "logs") + GetTypeName() + ".log";
        engineParameters_[EP_FULL_SCREEN] = false;
        engineParameters_[EP_WINDOW_WIDTH] = 800;
        engineParameters_[EP_WINDOW_HEIGHT] = 600;
        engineParameters_[EP_RESOURCE_PATHS] = "GameData;Data;CoreData";
    }

    void Start()
    {
        InitGlobal(context_);

        CreateScene();
        SetupViewport();
        SubscribeToEvents();
    }

    void SetupViewport()
    {
        Camera* camera = cameraNode_->GetComponent<Camera>();
        SharedPtr<Viewport> viewport(new Viewport(context_, scene_, camera));
        renderer->SetViewport(0, viewport);
    }

    void ApplyGameState(StringHash eventType, VariantMap& eventData)
    {
        /*if (GLOBAL->gameState_ == GLOBAL->neededGameState_)
            return;

        if (GLOBAL->gameState_ == GS_INIT)
        {

        }

        GLOBAL->gameState_ = GLOBAL->neededGameState_;
        UpdateUIVisibility();*/
    }

    void UpdateUIVisibility()
    {
        /*String tag;
        if (GLOBAL->gameState_ == GS_INIT)             tag = "Init";
        else if (GLOBAL->gameState_ == GS_GAMEPLAY)    tag = "Gameplay";

        auto uiElements = UI_ROOT->GetChildren();
        for (auto i = uiElements.Begin(); i != uiElements.End(); i++)
        {
            auto element = *i;
            element->SetVisible(element->HasTag(tag));
        }*/
    }

    void CreateScene()
    {
        scene_ = new Scene(context_);
        File levelFile(context_, fileSystem->GetProgramDir() + "GameData/Scenes/Level.xml");
        scene_->LoadXML(levelFile);

        cameraNode_ = scene_->CreateChild("Camera");
        cameraNode_->CreateComponent<Camera>();
        cameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));

        charNode_ = CreateCharacter();
    }

    void MoveCamera(float timeStep)
    {
        // Не двигаем камеру, если есть активный UI-элемент (консоль).
        if (ui->GetFocusElement())
            return;

    }

    void SubscribeToEvents()
    {
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Game, HandleUpdate));
        SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Game, HandlePhysicsPreStep));
        SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Game, HandlePostRenderUpdate));
        SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Game, HandlePostUpdate));
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace Update;
        float timeStep = eventData[P_TIMESTEP].GetFloat();

        if (input->GetKeyPress(KEY_F1))
            console->Toggle();

        if (input->GetKeyPress(KEY_F2))
            debugHud->ToggleAll();

        IntVector2 mouseMove = input->GetMouseMove();
        yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
        pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
        pitch_ = Clamp(pitch_, -90.0f, 90.0f);

        charNode_->SetRotation(Quaternion(0.0f, yaw_, 0.0f));
    }

    Node* CreateCharacter()
    {
        Node* charNode = scene_->CreateChild();
        charNode->SetPosition(Vector3(0.0f, 4.0f, 20.0f));

        StaticModel* charObject = charNode->CreateComponent<StaticModel>();
        charObject->SetModel(cache->GetResource<Model>("Models/Jack.mdl"));
        charObject->SetCastShadows(true);

        CharacterController* controller = charNode->CreateComponent<CharacterController>();
        controller->CreatePhysComponents(1.9f, 0.5f);
        return charNode;
    }

    //float MOVE_SPEED = 20.0f;
    const float MOUSE_SENSITIVITY = 0.1f;

    void HandlePhysicsPreStep(StringHash eventType, VariantMap& eventData)
    {
    }


    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
    {
        // If draw debug mode is enabled, draw physics debug geometry. Use depth test to make the result easier to interpret
        //if (drawDebug_)
            scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
    }

    void HandlePostUpdate(StringHash eventType, VariantMap& eventData)
    {
        // Get camera lookat dir from character yaw + pitch
        Quaternion rot = charNode_->GetRotation();
        Quaternion dir = rot * Quaternion(pitch_, Vector3::RIGHT);

        Vector3 aimPoint = charNode_->GetPosition() + rot * Vector3(0.0f, 1.9f, 0.0f);

        Vector3 rayDir = dir * Vector3::BACK;
        float rayDistance = 5.0f;
        PhysicsRaycastResult result;
        scene_->GetComponent<PhysicsWorld>()->RaycastSingle(result, Ray(aimPoint, rayDir), rayDistance, 2);
        if (result.body_)
            rayDistance = Min(rayDistance, result.distance_);
        rayDistance = Clamp(rayDistance, 1.0f, 5.0f);

        cameraNode_->SetPosition(aimPoint + rayDir * rayDistance);
        cameraNode_->SetRotation(dir);
    }
    
};

URHO3D_DEFINE_APPLICATION_MAIN(Game)
