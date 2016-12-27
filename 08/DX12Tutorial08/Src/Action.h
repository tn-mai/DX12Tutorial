/**
* @file Action.h
*/
#ifndef DX12TUTORIAL_SRC_ACTION_H_
#define DX12TUTORIAL_SRC_ACTION_H_
#include <DirectXMath.h>
#include <stdint.h>
#include <vector>

namespace Sprite {
struct Sprite;
} // namespace Sprite

namespace Action {

struct List;
enum class Type;
enum InterporationType;

struct Point {
	DirectX::XMFLOAT2 pos;
	float t;
	bool operator<(const Point& p) const { return t < p.t; }
};

class ActionController
{
public:
	ActionController() = delete;
	ActionController(const List& l, uint32_t no = 0);
	void Update(float delta, Sprite::Sprite*);
	void SetSeqIndex(uint32_t no);
	void SetManualMove(float degree, float speed);
	void SetManualAccel(float degree, float accel);
	void SetTime(float time);
	float GetCurrentTime() const { return currentTime; }
	float GetTotalTime() const { return totalTime; }

private:
	void Init();
	void UpdateSub(float delta, Sprite::Sprite*);

private:
	const List& list;
	uint32_t seqIndex;
	uint32_t dataIndex;
	Type type;

	float totalTime;
	float currentTime;

	DirectX::XMFLOAT2 move;
	DirectX::XMFLOAT2 accel;
	struct PathParam {
		std::vector<Point> cp;
		InterporationType type;
	} path;
};

const List& GetList();

} // namespace Action

#endif // DX12TUTORIAL_SRC_ACTION_H_