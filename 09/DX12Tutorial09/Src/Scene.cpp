/**
* @file Scene.cpp
*/
#include "Scene.h"
#include "Graphics.h"
#include <algorithm>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace Scene {

/**
*
*/
bool Context::Initialize(const Transition* transitionList, size_t transitionCount, const Creator* creatorList, size_t creatorCount)
{
	transitionMap.assign(transitionList, transitionList + transitionCount);
	std::stable_sort(transitionMap.begin(), transitionMap.end(), [](const Transition& lhs, const Transition& rhs) { return lhs.currentScene < rhs.currentScene; });
	const Creator* const end = creatorList + creatorCount;
	for (const Creator* itr = creatorList; itr != end; ++itr) {
		creatorMap.insert(std::make_pair(itr->id, itr->func));
	}
	return true;
}

/**
*
*/
bool Context::Start(int startSceneId)
{
	const MapType::iterator creator = creatorMap.find(startSceneId);
	if (creator != creatorMap.end()) {
		LoadScene(creator->first, creator->second);
		return true;
	}
	return false;
}

/**
*/
struct LessCurrentSceneId
{
	bool operator()(const Transition& lhs, int rhs) const { return lhs.currentScene < rhs; }
	bool operator()(int lhs, const Transition& rhs) const { return lhs < rhs.currentScene; }
	bool operator()(const Transition& lhs, const Transition& rhs) const { return lhs.currentScene < rhs.currentScene; }
};

/**
*
*/
void Context::Update(double delta)
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
			case TransitionType::Jump: {
				const MapType::iterator creator = creatorMap.find(itr->trans.nextScene);
				if (creator != creatorMap.end()) {
					UnloadScene();
					LoadScene(creator->first, creator->second);
				}
				break;
			}
			case TransitionType::Push: {
				const MapType::iterator creator = creatorMap.find(itr->trans.nextScene);
				if (creator != creatorMap.end()) {
					sceneStack.back().p->Pause();
					LoadScene(creator->first, creator->second);
				}
				break;
			}
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
void Context::Draw(Graphics::Graphics& graphics) const
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
void Context::LoadScene(int id, Creator::Func func)
{
	sceneStack.push_back({ id, func() });
	sceneStack.back().p->Load();
	sceneStack.back().p->status = Scene::StatusCode::Runnable;
}

/**
*
*/
void Context::UnloadScene()
{
	sceneStack.back().p->Unload();
	sceneStack.back().p->status = Scene::StatusCode::Stopped;
	sceneStack.pop_back();
	Graphics::Graphics::Get().WaitForGpu();
}

} // namespace Scene