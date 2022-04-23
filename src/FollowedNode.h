#ifndef __FOLLOWED_NODE_H__
#define __FOLLOWED_NODE_H__

#include "cocos2d.h"

USING_NS_CC;

class FollowedNode : public Node {
public:
	enum class TrailMotionType {
		STATIC,
		MOVE_OUT,
		MOVE_IN,
	};

	FollowedNode();
	~FollowedNode();

	void update(float dt) override;
	void setParent(Node* parent) override;

	static FollowedNode* create(Node* container, std::function<Node*()> createTrailNode, int trailSpawnFrequency, float trailLifeTime, float minimumSpawnDisplacement);
	bool init(Node* container, std::function<Node*()> createTrailNode, int trailSpawnFrequency, float trailLifeTime, float minimumSpawnDisplacement);
	
	void setStartPosition(const Vec2& startPosition);
	void moveTrail(Node* trail, const Vec2& initialPosition, const Vec2& finalPosition);
	void fadeTrail(Node* trail);

	int getTrailSpawnFrequency();
	void setTrailSpawnFrequency(int trailSpawnFrequency);
	int getTrailLifeTime();
	void setTrailLifeTime(int trailLifeTime);
	float getMinimumSpawnDisplacement();
	void setMinimumSpawnDisplacement(float minimumSpawnDisplacement);
	float getFadeDurationFraction();
	void setFadeDurationFraction(float fadeDurationFraction);
	TrailMotionType getTrailMotionType();
	void setTrailMotionType(TrailMotionType trailMotionType);

private:
	Node* m_container{ nullptr };
	std::vector<Node*> m_trails{};
	int m_trailSpawnFrequency{ 0 };
	float m_trailLifeTime{ 0.f };
	float m_minimumSpawnDisplacement{ 0.f };
	float m_fadeDurationFraction{ 0.3f };
	Vec2 m_lastSpawnPosition{ Vec2::ZERO };
	unsigned int m_frameCount{ 0 };
	unsigned int m_currentTrailIndex{ 0 };
	TrailMotionType m_trailMotionType{ TrailMotionType::STATIC };
};

#endif __FOLLOWED_NODE_H__ // __FOLLOWED_NODE_H__