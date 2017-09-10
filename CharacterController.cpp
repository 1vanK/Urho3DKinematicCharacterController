#include <Bullet/btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <Bullet/BulletDynamics/Character/btKinematicCharacterController.h>

#include "CharacterController.h"
#include "Global.h"

#undef new

const int CTRL_FORWARD = 1;
const int CTRL_BACK = 2;
const int CTRL_LEFT = 4;
const int CTRL_RIGHT = 8;
const int CTRL_JUMP = 16;

// Функция копирует значения X и Z из вектора from в вектор target.
// Эта функция используется, чтобы изменить горизонтальную скорость объекта, но не затронуть вертикальную.
/*static void SetXZ(Vector3& target, const Vector3& from)
{
    target.x_ = from.x_;
    target.z_ = from.z_;
}*/

CharacterController::CharacterController(Context* context) : LogicComponent(context)
{
    SetUpdateEventMask(USE_FIXEDUPDATE | USE_UPDATE);

}

CharacterController::~CharacterController()
{
    
}

void CharacterController::RegisterObject(Context* context)
{
    context->RegisterFactory<CharacterController>();
}

void CharacterController::FixedUpdate(float timeStep)
{
    const float MOVE_SPEED = 10.0f;

    Vector3 dir;

    if (controls_.IsDown(CTRL_FORWARD))
        dir += Vector3::FORWARD;
    if (controls_.IsDown(CTRL_BACK))
        dir += Vector3::BACK;
    if (controls_.IsDown(CTRL_LEFT))
        dir += Vector3::LEFT;
    if (controls_.IsDown(CTRL_RIGHT))
        dir += Vector3::RIGHT;

    if (controls_.IsDown(CTRL_JUMP))
    {
        if (bulletController_->onGround())
            bulletController_->jump(btVector3(0, 6, 0));
    }

    if (dir.LengthSquared() > 0.0f)
    {
        dir.Normalize();

        dir = node_->GetWorldRotation() * dir;

        if (bulletController_->onGround())
            bulletController_->setWalkDirection(ToBtVector3(dir * timeStep * MOVE_SPEED));
        else
            bulletController_->setWalkDirection(ToBtVector3(dir * timeStep * MOVE_SPEED * 0.5f));
    }
    else
    {
        bulletController_->setWalkDirection(btVector3(0, 0, 0));
    }

    btTransform t;
    t = bulletController_->getGhostObject()->getWorldTransform();
    Vector3 newPos = ToVector3(t.getOrigin()) + Vector3::DOWN * height_ * 0.5f;

    node_->SetWorldPosition(newPos);
}

void CharacterController::Update(float timeStep)
{
    // Может быть перенести это в общий класс, чтобы был доступ окна настроек клавиш
    controls_.Reset();

    controls_.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
    controls_.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
    controls_.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
    controls_.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
    controls_.Set(CTRL_JUMP, input->GetKeyDown(KEY_SPACE));

    btTransform t;
    t = bulletController_->getGhostObject()->getWorldTransform();
    t.setRotation(ToBtQuaternion(node_->GetRotation()));
    bulletController_->getGhostObject()->setWorldTransform(t);
}

void CharacterController::CreatePhysComponents(float height, float diameter)
{
 //   RigidBody* body = node_->CreateComponent<RigidBody>();
  //  body->SetKinematic(true);
  //  body->SetMass(1.0f);

   // CollisionShape* shape = node_->CreateComponent<CollisionShape>();
    //shape->SetCapsule(diameter, height, Vector3(0.0f, height * 0.5f, 0.0f));
    
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(0, 10, 4));
    startTransform.setRotation(ToBtQuaternion(Quaternion(90, 0, 0)));

    btConvexShape* capsule = new btCapsuleShape(diameter * 0.5, height - diameter);
    height_ = height;
    diameter_ = diameter;

    btDiscreteDynamicsWorld* world = GetScene()->GetComponent<PhysicsWorld>()->GetWorld();

    ghostObject_ = new btPairCachingGhostObject();
    ghostObject_->setWorldTransform(startTransform);
    world->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    ghostObject_->setCollisionShape(capsule);
    ghostObject_->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
    bulletController_ = new btKinematicCharacterController(ghostObject_, capsule, 0.3f, btVector3(0, 0, 1));
    bulletController_->setGravity(world->getGravity());

    world->addCollisionObject(ghostObject_, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::AllFilter);
    world->addAction(bulletController_);
    bulletController_->setMaxJumpHeight(1.5);
}

void CharacterController::Start()
{

}