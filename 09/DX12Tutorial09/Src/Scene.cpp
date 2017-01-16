/**
* @file Scene.cpp
*/
#include "Scene.h"
#include "Graphics.h"
#include <algorithm>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace Scene {

namespace /* unnamed */ {

/**
*/
struct LessSceneId
{
	bool operator()(const Creator& lhs, int rhs) const { return lhs.id < rhs; }
	bool operator()(int lhs, const Creator& rhs) const { return lhs < rhs.id; }
	bool operator()(const Creator& lhs, const Creator& rhs) const { return lhs.id < rhs.id; }
};

/**
*/
struct LessCurrentSceneId
{
	bool operator()(const Transition& lhs, int rhs) const { return lhs.currentScene < rhs; }
	bool operator()(int lhs, const Transition& rhs) const { return lhs < rhs.currentScene; }
	bool operator()(const Transition& lhs, const Transition& rhs) const { return lhs.currentScene < rhs.currentScene; }
};

} // unnamed namespace

/**
*
*/
bool TransitionController::Initialize(const Transition* transitionList, size_t transitionCount, const Creator* creatorList, size_t creatorCount)
{
	transitionMap.assign(transitionList, transitionList + transitionCount);
	std::stable_sort(transitionMap.begin(), transitionMap.end(), [](const Transition& lhs, const Transition& rhs) { return lhs.currentScene < rhs.currentScene; });
	
	creatorMap.assign(creatorList, creatorList + creatorCount);
	std::stable_sort(creatorMap.begin(), creatorMap.end(), [](const Creator& lhs, const Creator& rhs) { return lhs.id < rhs.id; });
	return true;
}

/**
*
*/
bool TransitionController::Start(int startSceneId)
{
	if (const Creator* creator = FindCreator(startSceneId)) {
		LoadScene(creator);
		return true;
	}
	return false;
}

/**
*
*/
void TransitionController::Update(double delta)
{
	auto itrEndPausedScene = sceneStack.end() - 1;
	for (auto itr = sceneStack.begin(); itr != itrEndPausedScene; ++itr) {
		(*itr).p->UpdateForPause(delta);
	}
	const int exitCode = sceneStack.back().p->Update(delta);
	if (exitCode != Scene::ExitCode_Continue) {
		const auto range = std::equal_range(transitionMap.begin(), transitionMap.end(), sceneStack.back().id, LessCurrentSceneId());
		const auto itr = std::find_if(range.first, range.second,
			[exitCode](const Transition& trans) { return trans.trans.exitCode == exitCode; });
		if (itr != range.second) {
			switch (itr->trans.type) {
			case TransitionType::Jump:
				if (const Creator* creator = FindCreator(itr->trans.nextScene)) {
					UnloadScene();
					LoadScene(creator);
				}
				break;
			case TransitionType::Push:
				if (const Creator* creator = FindCreator(itr->trans.nextScene)) {
					sceneStack.back().p->Pause();
					LoadScene(creator);
				}
				break;
			case TransitionType::Pop:
				UnloadScene();
				sceneStack.back().p->Resume();
				break;
			}
		}
	}
	Graphics::Graphics::Get().texMap.GC();
}

/**
*
*/
void TransitionController::Draw(Graphics::Graphics& graphics) const
{
	for (const auto& e : sceneStack) {
		if (e.p->GetState() == Scene::StatusCode::Runnable) {
			e.p->Draw(graphics);
		}
	}
}

/**
*
*/
const Creator* TransitionController::FindCreator(int sceneId) const
{
	const auto itr = std::lower_bound(creatorMap.begin(), creatorMap.end(), sceneId, LessSceneId());
	if (itr == creatorMap.end() || itr->id != sceneId) {
		return nullptr;
	}
	return &*itr;
}

/**
*
*/
void TransitionController::LoadScene(const Creator* creator)
{
	sceneStack.push_back({ creator->id, creator->func() });
	sceneStack.back().p->Load();
	sceneStack.back().p->status = Scene::StatusCode::Runnable;
}

/**
*
*/
void TransitionController::UnloadScene()
{
	sceneStack.back().p->Unload();
	sceneStack.back().p->status = Scene::StatusCode::Stopped;
	sceneStack.pop_back();
	Graphics::Graphics::Get().WaitForGpu();
}

} // namespace Scene