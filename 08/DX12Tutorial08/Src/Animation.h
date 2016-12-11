/**
* @file Animation.h
**/
#ifndef DX12TUTORIAL_SRC_ANIMATION_H_
#define DX12TUTORIAL_SRC_ANIMATION_H_
#include <vector>
#include <stdint.h>

struct AnimationData
{
	uint32_t cell;
	float time;
};

struct AnimationSequence
{
	std::vector<AnimationData> sequence;
};

struct AnimationList
{
	std::vector<AnimationSequence> list;
};

class AnimationController
{
public:
	AnimationController();
	void SetList(const AnimationList& list);
	void SetSeqNo(uint32_t no);
	void Update(double delta);
	const uint32_t GetCellIndex() const;
	const size_t GetSeqNum() const { return list ? list->list.size() : 0; }

private:
	const AnimationList* list;
	uint32_t seqNo;
	uint32_t index;
	double time;
};

const AnimationList& GetAnimationList();

#endif // DX12TUTORIAL_SRC_ANIMATION_H_