#include "FollowedNode.h"

const int ASSUMED_FRAME_RATE = 60;
const int MAX_TRAIL_SPAWN_FREQUENCY = 30;

FollowedNode::FollowedNode() {
}

FollowedNode::~FollowedNode() {
	for (Node* trail : m_trails) {
		CC_SAFE_RELEASE(trail);
	}

	m_trails.clear();
}

FollowedNode* FollowedNode::create(Node* container, std::function<Node*()> createTrailNode, int trailSpawnFrequency, float trailLifeTime, float minimumSpawnDisplacement) {
	FollowedNode* followedNode = new (std::nothrow) FollowedNode();
	if (followedNode && followedNode->init(container, createTrailNode, trailSpawnFrequency, trailLifeTime, minimumSpawnDisplacement)) {
		followedNode->autorelease();
		return followedNode;
	}

	CC_SAFE_DELETE(followedNode);
	return nullptr;
}

bool FollowedNode::init(Node* container, std::function<Node*()> createTrailNode, int trailSpawnFrequency, float trailLifeTime, float minimumSpawnDisplacement) {
	CCASSERT(container, "container is null");
	CCASSERT(trailSpawnFrequency >= 0, "trailSpawnFrequency should be greate than zero");
	CCASSERT(trailLifeTime >= 0 , "trailLifeTime should be positive");
	CCASSERT(minimumSpawnDisplacement >= 0, "minimumSpawnDisplacement should be positive");

	m_container = container;
	m_trailSpawnFrequency = trailSpawnFrequency;
	m_trailLifeTime = trailLifeTime;
	m_minimumSpawnDisplacement = minimumSpawnDisplacement;

	int trailCount = m_trailSpawnFrequency * (m_trailLifeTime + 1);
	if (m_trailSpawnFrequency == 0) {
		trailCount = MAX_TRAIL_SPAWN_FREQUENCY * (m_trailLifeTime + 1);
	}
	for (int i = 0; i < trailCount; i++) {
		Node* trail = createTrailNode();
		if (trail) {
			m_trails.push_back(trail);
			trail->setVisible(false);
			trail->setCascadeOpacityEnabled(true);
			trail->retain();
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

void FollowedNode::update(float dt) {
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
		if (m_trailMotionType == TrailMotionType::MOVE_OUT) {
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

void FollowedNode::setParent(Node* parent) {
	Node::setParent(parent);

	for (Node* trail : m_trails) {
		this->getParent()->addChild(trail);
		trail->release();
	}
}

void FollowedNode::setStartPosition(const Vec2& startPosition) {
	this->setPosition(startPosition);
	m_lastSpawnPosition = startPosition;
}

int FollowedNode::getTrailSpawnFrequency() {
	return m_trailSpawnFrequency;
}

void FollowedNode::setTrailSpawnFrequency(int trailSpawnFrequency) {
	m_trailSpawnFrequency = trailSpawnFrequency;
}

int FollowedNode::getTrailLifeTime() {
	return m_trailLifeTime;
}

void FollowedNode::setTrailLifeTime(int trailLifeTime) {
	m_trailLifeTime = trailLifeTime;
}

float FollowedNode::getMinimumSpawnDisplacement() {
	return m_minimumSpawnDisplacement;
}

void FollowedNode::setMinimumSpawnDisplacement(float minimumSpawnDisplacement) {
	m_minimumSpawnDisplacement = minimumSpawnDisplacement;
}

float FollowedNode::getFadeDurationFraction() {
	return m_fadeDurationFraction;
}

void FollowedNode::setFadeDurationFraction(float fadeDurationFraction) {
	CCASSERT(fadeDurationFraction >= 0 && fadeDurationFraction <= 1, "fadeDurationFraction should be in the range [0,1]");
	m_fadeDurationFraction = fadeDurationFraction;
}

FollowedNode::TrailMotionType FollowedNode::getTrailMotionType() {
	return m_trailMotionType;
}

void FollowedNode::setTrailMotionType(TrailMotionType trailMotionType) {
	m_trailMotionType = trailMotionType;
}

void FollowedNode::moveTrail(Node* trail, const Vec2& initialPosition, const Vec2& finalPosition) {
	trail->setPosition(initialPosition);
	trail->runAction(MoveTo::create(m_trailLifeTime, finalPosition));
}

void FollowedNode::fadeTrail(Node* trail) {
	trail->setOpacity(255);
	trail->runAction(Sequence::create(DelayTime::create(m_trailLifeTime * (1 - m_fadeDurationFraction)), FadeOut::create(m_trailLifeTime * m_fadeDurationFraction), nullptr));
}