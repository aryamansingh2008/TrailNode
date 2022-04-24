#ifndef __TRAIL_NODE_H__
#define __TRAIL_NODE_H__

#include "cocos2d.h"

USING_NS_CC;

class TrailNode : public Node {
public:
	enum class TrailMotionType {
		STATIC,
		MOVE_OUT,
		MOVE_IN,
	};

	TrailNode();
	~TrailNode();

	void update(float dt) override;
	void setParent(Node* parent) override;

	/**
	 * Creates an auto managed object of type TrailNode.
	 */
	static TrailNode* create(Node* container, std::function<Node* ()> createTrail, int trailSpawnFrequency, float trailLifeTime, float minimumSpawnDisplacement);

	/**
	 * Called by create method to initialize the member variables. Can be called to change properties of TrailNode object.
	 */
	bool init(Node* container, std::function<Node* ()> createTrail, int trailSpawnFrequency, float trailLifeTime, float minimumSpawnDisplacement);

	/**
	 * Should be called to set the TrailNode object at its initial position.  Any repositioning after this call might lead to creation of trails depending on the trail frequency and minimum displacement value.
	 */
	void setStartPosition(const Vec2& startPosition);

	/**
	 * TrailSpawnFrequency: the number of trails spawning per second. 0 value means the spawning becomes independent of the time.
	 */
	int getTrailSpawnFrequency();
	void setTrailSpawnFrequency(int trailSpawnFrequency);

	/**
	 * TrailLifeTime: the duration in seconds for which the trail is visble on the screen.
	 */
	float getTrailLifeTime();
	void setTrailLifeTime(float trailLifeTime);

	/**
	 * MinimumSpawnDistance: the minimum amount of displacement of the TrailNode which triggers the spawn of trails.
	 */
	float getMinimumSpawnDisplacement();
	void setMinimumSpawnDisplacement(float minimumSpawnDisplacement);

	/**
	 * TrailFadeDurationFraction: the decimal fraction of the trail lifetime that is involved in the fade out animation.
	 * Should be in the range [0, 1]
	 */
	float getTrailFadeDurationFraction();
	void setTrailFadeDurationFraction(float fadeDurationFraction);

	/**
	 * TrailMotionType: the trail animation type.
	 */
	TrailMotionType getTrailMotionType();
	void setTrailMotionType(TrailMotionType trailMotionType);

private:
	Node* m_container{ nullptr };
	std::vector<Node*> m_trails{};
	int m_trailSpawnFrequency{ 0 };
	float m_trailLifeTime{ 0.f };
	float m_minimumSpawnDisplacement{ 0.f };
	float m_trailFadeDurationFraction{ 0.3f };
	Vec2 m_lastSpawnPosition{ Vec2::ZERO };
	unsigned int m_frameCount{ 0 };
	unsigned int m_currentTrailIndex{ 0 };
	TrailMotionType m_trailMotionType{ TrailMotionType::STATIC };

private:
	void clearTrails();
	void moveTrail(Node* trail, const Vec2& initialPosition, const Vec2& finalPosition);
	void fadeTrail(Node* trail);
};

#endif __TRAIL_NODE_H__ // __TRAIL_NODE_H__