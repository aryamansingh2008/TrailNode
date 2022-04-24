#include "TrailNode.h"

const int ASSUMED_FRAME_RATE = 60;
const int MAX_TRAIL_SPAWN_FREQUENCY = 30;

TrailNode::TrailNode() {
}

TrailNode::~TrailNode() {
}

TrailNode* TrailNode::create(Node* container, std::function<Node* ()> createTrail, int trailSpawnFrequency, float trailLifeTime, float minimumSpawnDisplacement) {
	TrailNode* trailNode = new (std::nothrow) TrailNode();
	if (trailNode && trailNode->init(container, createTrail, trailSpawnFrequency, trailLifeTime, minimumSpawnDisplacement)) {
		trailNode->autorelease();
		return trailNode;
	}

	CC_SAFE_DELETE(trailNode);
	return nullptr;
}

bool TrailNode::init(Node* container, std::function<Node* ()> createTrail, int trailSpawnFrequency, float trailLifeTime, float minimumSpawnDisplacement) {
	CCASSERT(container, "container is null");
	CCASSERT(trailSpawnFrequency >= 0, "trailSpawnFrequency should be greate than zero");
	CCASSERT(trailLifeTime >= 0, "trailLifeTime should be positive");
	CCASSERT(minimumSpawnDisplacement >= 0, "minimumSpawnDisplacement should be positive");

	m_container = container;
	m_trailSpawnFrequency = trailSpawnFrequency;
	m_trailLifeTime = trailLifeTime;
	m_minimumSpawnDisplacement = minimumSpawnDisplacement;

	this->clearTrails();
	this->setCascadeOpacityEnabled(true);
	this->removeAllChildren();

	int trailCount = m_trailSpawnFrequency * (int)ceil(m_trailLifeTime);
	if (m_trailSpawnFrequency == 0) {
		trailCount = MAX_TRAIL_SPAWN_FREQUENCY * (int)ceil(m_trailLifeTime);
	}
	for (int i = 0; i < trailCount; i++) {
		Node* trail = createTrail();
		if (trail) {
			m_trails.push_back(trail);
			trail->setVisible(false);
			this->addChild(trail);
		}
	}


	if (m_container && m_trails.size() == trailCount) {
		this->addChild(m_container);
		m_container->setPosition(Vec2::ZERO);
		this->scheduleUpdate();
		return true;
	}

	return false;
}

void TrailNode::update(float dt) {
	int spawnFrame = 1;
	if (m_trailSpawnFrequency > 0) {
		spawnFrame = ASSUMED_FRAME_RATE / m_trailSpawnFrequency;
	}
	if (m_frameCount % spawnFrame == 0 && this->getPosition().distance(m_lastSpawnPosition) >= m_minimumSpawnDisplacement) {
		Node* trail = m_trails.at(m_currentTrailIndex);
		trail->stopAllActions();
		trail->setVisible(true);
		const Vec2& initialPosition = this->getPosition();
		const Vec2& finalPosition = m_lastSpawnPosition.lerp(this->getPosition(), 0.9f);
		if (m_trailMotionType == TrailMotionType::STATIC) {
			trail->setPosition(initialPosition);
		} else if (m_trailMotionType == TrailMotionType::MOVE_OUT) {
			this->moveTrail(trail, initialPosition, finalPosition);
		} else if (m_trailMotionType == TrailMotionType::MOVE_IN) {
			this->moveTrail(trail, finalPosition, initialPosition);
		}
		this->fadeTrail(trail);

		m_frameCount = 0;
		m_lastSpawnPosition = this->getPosition();
		m_currentTrailIndex = (m_currentTrailIndex + 1) % m_trails.size();
	}

	m_frameCount++;
}

void TrailNode::setParent(Node* parent) {
	Node::setParent(parent);

	for (Node* trail : m_trails) {
		trail->retain();
		trail->removeFromParentAndCleanup(true);
		this->getParent()->addChild(trail);
		trail->release();
	}
}

void TrailNode::setStartPosition(const Vec2& startPosition) {
	this->setPosition(startPosition);
	m_lastSpawnPosition = startPosition;
}

int TrailNode::getTrailSpawnFrequency() {
	return m_trailSpawnFrequency;
}

void TrailNode::setTrailSpawnFrequency(int trailSpawnFrequency) {
	m_trailSpawnFrequency = trailSpawnFrequency;
}

float TrailNode::getTrailLifeTime() {
	return m_trailLifeTime;
}

void TrailNode::setTrailLifeTime(float trailLifeTime) {
	m_trailLifeTime = trailLifeTime;
}

float TrailNode::getMinimumSpawnDisplacement() {
	return m_minimumSpawnDisplacement;
}

void TrailNode::setMinimumSpawnDisplacement(float minimumSpawnDisplacement) {
	m_minimumSpawnDisplacement = minimumSpawnDisplacement;
}

float TrailNode::getTrailFadeDurationFraction() {
	return m_trailFadeDurationFraction;
}

void TrailNode::setTrailFadeDurationFraction(float trailFadeDurationFraction) {
	CCASSERT(trailFadeDurationFraction >= 0 && trailFadeDurationFraction <= 1, "fadeDurationFraction should be in the range [0,1]");
	m_trailFadeDurationFraction = trailFadeDurationFraction;
}

TrailNode::TrailMotionType TrailNode::getTrailMotionType() {
	return m_trailMotionType;
}

void TrailNode::setTrailMotionType(TrailMotionType trailMotionType) {
	m_trailMotionType = trailMotionType;
}

void TrailNode::moveTrail(Node* trail, const Vec2& initialPosition, const Vec2& finalPosition) {
	trail->runAction(MoveTo::create(m_trailLifeTime, finalPosition));
}

void TrailNode::fadeTrail(Node* trail) {
	trail->setOpacity(255);
	trail->runAction(Sequence::create(DelayTime::create(m_trailLifeTime * (1 - m_trailFadeDurationFraction)), FadeOut::create(m_trailLifeTime * m_trailFadeDurationFraction), nullptr));
}

void TrailNode::clearTrails() {
	for (Node* trail : m_trails) {
		trail->removeFromParentAndCleanup(true);
	}

	m_trails.clear();
}