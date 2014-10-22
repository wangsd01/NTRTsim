#include "cordeAnchor.h"

#include "cordeCollisionObject.h"

#include "tgcreator/tgUtil.h"

// The Bullet Physics library
#include "BulletDynamics/Dynamics/btRigidBody.h"

// The C++ Standard Library
#include <cassert>
#include <iostream>

cordeAnchor::cordeAnchor()
{
}

cordeAnchor::cordeAnchor(btRigidBody * body,
               CordeModel::CordePositionElement* element,
               CordeModel::CordeQuaternionElement* qElement,
               btVector3 worldPos) :
  attachedBody(body),
  m_attachedElement(element),
  m_attachedQuaternion(qElement),
  // Find relative position
  // This should give relative position in a default orientation.
  attachedRelativeOriginalPosition(attachedBody->getWorldTransform().inverse() *
                   worldPos)
{
	m_attachedElement->isAnchor = true;
	assert(body);
	assert(element);
	assert(qElement);
}

cordeAnchor::~cordeAnchor()
{
	m_attachedElement->isAnchor = false;
	
    // World should delete this
    attachedBody = NULL;
    m_attachedElement = NULL;
}

// This returns current position relative to the rigidbody.
btVector3 cordeAnchor::getRelativePosition()
{
    const btTransform tr = attachedBody->getWorldTransform();
    const btVector3 worldPos = tr * attachedRelativeOriginalPosition;
    return worldPos-this->attachedBody->getCenterOfMassPosition();
}

btVector3 cordeAnchor::getWorldPosition()
{
    const btTransform tr = attachedBody->getWorldTransform();
    return tr * attachedRelativeOriginalPosition;
}

void cordeAnchor::solve(const double dt)
{
	assert(attachedBody);
	assert(m_attachedElement);

	const btScalar	idt = 1.0 / dt;
//#define SOLVE_VELOCITY	
#ifndef SOLVE_VELOCITY	
	// Difference between pos_new and getRelativePosition
	const btVector3 rbPos = getWorldPosition();
	const btVector3 softPos = m_attachedElement->pos_new;


	const btVector3 posDiff = rbPos - softPos;
	
	const btScalar rbLength = attachedRelativeOriginalPosition.length();
	const btScalar softLength = (attachedBody->getCenterOfMassPosition() - softPos).length();
	
	// Apply forces based on mass ratio, that difference
	
	const btScalar rMass = attachedBody->getInvMass() > 0.0 ? 1.0 / attachedBody->getInvMass() : 0.0;
	const btScalar sMass = m_attachedElement->mass;

	btScalar massRatio;
	// No mass means immobile object, so softbody gets all the force
	if (rMass == 0)
	{
		massRatio	= 1.f;
	}
	else
	{
		// mr / (ms + mr)
		massRatio = (rMass) * 1.0 / (rMass + sMass);
	}
	
	//std::cout << m_attachedQuaternion->q << std::endl;
	//std::cout << attachedBody->getTotalTorque() << std::endl;
	
	// Normal * mass ratio * penetration distance
	btVector3 rSoft = posDiff * massRatio;
	btVector3 rRigid =  -posDiff  * (1.f - massRatio);

	btVector3 fSoft = pow(idt, 2.0) * sMass * rSoft;
	
	m_attachedElement->applyForce(fSoft);
	// Impulse instead of force
	btVector3 fRigid = pow(idt, 1.0) * rMass * rRigid + m_attachedElement->force * dt; // Transferring internal forces appears to be a little better
	//m_attachedQuaternion->tprime = btQuaternion(0.0, 0.0, 0.0, 0.0);


	attachedBody->activate();
	attachedBody->applyImpulse(fRigid, getRelativePosition());
	//attachedBody->applyTorqueImpulse(m_attachedQuaternion->torques * dt);
#else
	// Difference between pos_new and getRelativePosition
	const btVector3 rbVel = attachedBody->getVelocityInLocalPoint(getRelativePosition());
	const btVector3 softVel = m_attachedElement->vel_new;


	const btVector3 velDiff = rbVel - softVel;
	
	// Apply forces based on mass ratio, that difference
	
	const btScalar rMass = attachedBody->getInvMass() > 0.0 ? 1.0 / attachedBody->getInvMass() : 0.0;
	const btScalar sMass = m_attachedElement->mass;

	btScalar massRatio;
	// No mass means immobile object, so softbody gets all the force
	if (rMass == 0)
	{
		massRatio	= 1.f;
	}
	else
	{
		// mr / (ms + mr)
		massRatio = (rMass) * 1.0 / (rMass + sMass);
	}
	
	//std::cout << m_attachedQuaternion->q << std::endl;
	//std::cout << attachedBody->getTotalTorque() << std::endl;
	
	// Normal * mass ratio * penetration distance
	btVector3 vSoft = velDiff * massRatio;
	btVector3 vRigid =  -velDiff  * (1.f - massRatio);

	btVector3 fSoft = idt * sMass * vSoft;
	
	m_attachedElement->applyForce(fSoft);
	// Impulse instead of force
	btVector3 fRigid = rMass * vRigid;// + m_attachedElement->force * dt; // Transferring internal forces appears to be a little better
	//m_attachedQuaternion->tprime = btQuaternion(0.0, 0.0, 0.0, 0.0);


	attachedBody->activate();
	attachedBody->applyImpulse(fRigid, getRelativePosition());
	//attachedBody->applyTorqueImpulse(m_attachedQuaternion->torques * dt);
#endif	
	
}