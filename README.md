# Trail Node for cocos2dx v4.x

This provides functionality to add trails to moving objects.

## Integration:
1. Download and place src/ contents in the cocos2d/cocos/ folder in your cocos2d project.
> 
2. Edit cocos2d/cocos/CMakeLists.txt to include trail/CMakeLists.txt.
> 
3. Add COCOS_TRAIL_HEADER and COCOS_TRAIL_SRC to COCO_HEADER and COCOS_SRC respectively.

## Usage:

```cpp
Sprite* mainSprite = Sprite::create("sprite1.png");
FollowedNode* followedNode = FollowedNode::create(mainSprite, []()->Node*{
		return Sprite::create(sprite1.png");
}, 0, 1.f, 15.f);
```

The above lines of code create a FollowedNode with mainSprite and the trail objects are passed as a function returning the trail node. The rest of the arguments are frequency at which the trails spawn out of the object(0 here means the spawn should not depend upon time), lifetime of the trail objects(1.f means 1 second) and the minimum displacement of the object after which the trail should be created(15.f units of displacement with respect to the followedNode).

-----------------------

```cpp
followedNode->setStartPosition(Vec2(0, 0);
```

setStartPosition method is used to position the trail node at its initial position. Any repositioning of the followed node after this might lead to creation of trails depending on the trail frequency and minimum displacement value.

-----------------------

```cpp
followedNode->setTrailMotionType(FollowedNode::TrailMotionType::MOVE_IN);
```

setTrailMotionType can be used to change the trail opbjects animation. There are three supported TrailMotionType: STATIC, MOVE_IN and MOVE_OUT.

-----------------------

```cpp
followedNode->setFadeDurationFraction(0.5);
```

setFadeDurationFraction is used to set the percentage of lifetime of trail that should be used in the fading out animation. The value should be in the range [0, 1]
