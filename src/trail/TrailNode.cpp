#include "TrailNode.h"

const int ASSUMED_FRAME_RATE = 60;

TrailNode::TrailNode() {
}

TrailNode::~TrailNode() {
}

TrailNode* TrailNode::create(Node* container, std::function<Node* ()> createTrail, TrailMotionType trailMotionType, int trailSpawnFrequency, float trailLifeTime, int followTrailCount) {
	TrailNode* trailNode = new (std::nothrow) TrailNode();
	if (trailNode && trailNode->init(container, createTrail, trailMotionType, trailSpawnFrequency, trailLifeTime, followTrailCount)) {
		trailNode->autorelease();
		return trailNode;
	}

	CC_SAFE_DELETE(trailNode);
	return nullptr;
}

bool TrailNode::init(Node* container, std::function<Node* ()> createTrail, TrailMotionType trailMotionType, int trailSpawnFrequency, float trailLifeTime, int followTrailCount) {
	CCASSERT(container, "container should not be nulll");
	CCASSERT(trailSpawnFrequency >= 0, "trailSpawnFrequency should be greate than zero");
	CCASSERT(trailLifeTime >= 0, "trailLifeTime should be positive");
	CCASSERT(followTrailCount >= 0, "followTrailCount should be positive");

	m_createTrail = &createTrail;
	m_trailMotionType = trailMotionType;
	m_trailSpawnFrequency = trailSpawnFrequency;
	m_trailLifeTime = trailLifeTime;
	m_followTrailCount = followTrailCount;

	this->setCascadeOpacityEnabled(true);
	this->removeAllChildren();
	this->addChild(container);
	container->setPosition(Vec2::ZERO);
	this->scheduleUpdate();

	int trailCount = m_trailSpawnFrequency * (int)ceil(m_trailLifeTime);
	if (m_trailSpawnFrequency == 0) {
		trailCount = ASSUMED_FRAME_RATE * (int)ceil(m_trailLifeTime);
	}
	if (m_trailMotionType == TrailMotionType::FOLLOW) {
		trailCount = m_followTrailCount;
	}
	for (int i = 0; i < trailCount; i++) {
		Node* trail = createTrail();
		if (trail) {
			m_trails.push_back(trail);
			trail->setVisible(false);
			this->addChild(trail);
		} else {
			return false;
		}
	}

	return true;
}

void TrailNode::update(float dt) {
	if (this->shouldSpawnTrail()) {
		Node* trail = m_trails.at(m_currentTrailIndex);
		if (m_trailMotionType == TrailMotionType::FOLLOW) {
			trail->stopAllActions();
			trail->setVisible(true);
			trail->setOpacity(0);
			trail->runAction(FadeIn::create(0.3f));
			int positionHistoryIndex = m_positionHistory.size() - m_followTrailFramesSpan * (m_trailPositionIndex.size() + 1);
			trail->setPosition(m_positionHistory.at(positionHistoryIndex));
			m_trailPositionIndex[trail] = positionHistoryIndex;
			m_lastSpawnPosition = m_positionHistory.at(positionHistoryIndex);
			if (m_trailPositionIndex.size() == 1) {
				m_leadTrail = trail;
			}
		} else {
			trail->stopAllActions();
			trail->setVisible(true);
			const Vec2& initialPosition = this->getPosition();
			const Vec2& finalPosition = m_lastSpawnPosition.lerp(this->getPosition(), 0.9f);
			if (m_trailMotionType == TrailMotionType::STATIC) {
				trail->setPosition(initialPosition);
			} else if (m_trailMotionType == TrailMotionType::STATIC_MOVE_OUT) {
				this->moveTrail(trail, initialPosition, finalPosition);
			} else if (m_trailMotionType == TrailMotionType::STATIC_MOVE_IN) {
				this->moveTrail(trail, finalPosition, initialPosition);
			}
			this->fadeTrail(trail);
			m_lastSpawnPosition = this->getPosition();
		}

		m_frameCount = 0;
		m_currentTrailIndex = (m_currentTrailIndex + 1) % m_trails.size();
	}

	if (m_trailMotionType == TrailMotionType::FOLLOW) {
		size_t positionHistoryLength = m_trails.size() * m_followTrailFramesSpan;
		for (auto it = m_trailPositionIndex.begin(); it != m_trailPositionIndex.end(); it++) {
			Node* trail = it->first;
			trail->setPosition(m_positionHistory.at(it->second));
			if (m_positionHistory.size() < positionHistoryLength) {
				it->second++;
			}
		}

		m_positionHistory.push_back(this->getPosition());
		if (m_positionHistory.size() > positionHistoryLength) {
			m_positionHistory.pop_front();
		}

		const Vec2& windowEntryPosition = this->getPosition();
		if (m_trailWindowTrack[windowEntryPosition] == 0) {
			m_trailWindowTrack.clear();
		}
		m_trailWindowTrack[windowEntryPosition]++;

		if (m_trailPositionIndex.size() > 0) {
			if (m_trailWindowTrack[this->getPosition()] >= m_followTrailFramesSpan) {
				m_leadTrail->setVisible(false);
				m_trailPositionIndex.erase(m_leadTrail);
				m_trailWindowTrack.clear();

				int maxPositionIndex = INT_MIN;
				for (auto it = m_trailPositionIndex.begin(); it != m_trailPositionIndex.end(); it++) {
					if (it->second > maxPositionIndex) {
						maxPositionIndex = it->second;
						m_leadTrail = it->first;
					}
					it->second += m_followTrailFramesSpan;
				}
			}
		}
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

TrailNode::TrailMotionType TrailNode::getTrailMotionType() {
	return m_trailMotionType;
}

void TrailNode::setTrailMotionType(TrailMotionType trailMotionType) {
	m_trailMotionType = trailMotionType;
}

int TrailNode::getTrailSpawnFrequency() {
	return m_trailSpawnFrequency;
}

void TrailNode::setTrailSpawnFrequency(int trailSpawnFrequency) {
	m_trailSpawnFrequency = trailSpawnFrequency;

	if (m_trailMotionType != TrailMotionType::FOLLOW) {
		int trailCount = m_trailSpawnFrequency * (int)ceil(m_trailLifeTime);
		if (m_trailSpawnFrequency == 0) {
			trailCount = ASSUMED_FRAME_RATE * (int)ceil(m_trailLifeTime);
		}
		int currentToalTrails = m_trails.size();
		while (currentToalTrails < trailCount) {
			Node* trail = (*m_createTrail)();
			CCASSERT(trail, "Error creating Trail Object");

			m_trails.push_back(trail);
			trail->setVisible(false);
			this->addChild(trail);
			currentToalTrails++;
		}
	}
}

float TrailNode::getTrailLifeTime() {
	return m_trailLifeTime;
}

void TrailNode::setTrailLifeTime(float trailLifeTime) {
	m_trailLifeTime = trailLifeTime;

	if (m_trailMotionType != TrailMotionType::FOLLOW) {
		int trailCount = m_trailSpawnFrequency * (int)ceil(m_trailLifeTime);
		if (m_trailSpawnFrequency == 0) {
			trailCount = ASSUMED_FRAME_RATE * (int)ceil(m_trailLifeTime);
		}
		int currentToalTrails = m_trails.size();
		while (currentToalTrails < trailCount) {
			Node* trail = (*m_createTrail)();
			CCASSERT(trail, "Error creating Trail Object");

			m_trails.push_back(trail);
			trail->setVisible(false);
			this->addChild(trail);
			currentToalTrails++;
		}
	}
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

int TrailNode::getFollowTrailFramesSpan() {
	return m_followTrailFramesSpan;
}

void TrailNode::setFollowTrailFramesSpan(int followTrailFramesSpan) {
	m_followTrailFramesSpan = followTrailFramesSpan;
}

int TrailNode::getFollowTrailCount() {
	return m_followTrailCount;
}

void TrailNode::setFollowTrailCount(int followTrailCount) {
	m_followTrailCount = followTrailCount;

	if (m_trailMotionType == TrailMotionType::FOLLOW) {
		int currentToalTrails = m_trails.size();
		while (currentToalTrails < m_followTrailCount) {
			Node* trail = (*m_createTrail)();
			CCASSERT(trail, "Error creating Trail Object");

			m_trails.push_back(trail);
			trail->setVisible(false);
			this->addChild(trail);
			currentToalTrails++;
		}
	}
}

void TrailNode::moveTrail(Node* trail, const Vec2& initialPosition, const Vec2& finalPosition) {
	trail->setPosition(initialPosition);
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

bool TrailNode::shouldSpawnTrail() {
	int spawnFrame = 1;
	if (m_trailSpawnFrequency > 0) {
		spawnFrame = ASSUMED_FRAME_RATE / m_trailSpawnFrequency;
	}

	if (m_trailMotionType == TrailMotionType::FOLLOW) {
		return m_positionHistory.size() > 0 && m_positionHistory.back() != this->getPosition() && m_frameCount % m_followTrailFramesSpan == 0 && m_trailPositionIndex.size() < m_followTrailCount;
	}

	return m_frameCount % spawnFrame == 0 && this->getPosition().distance(m_lastSpawnPosition) >= m_minimumSpawnDisplacement;
}