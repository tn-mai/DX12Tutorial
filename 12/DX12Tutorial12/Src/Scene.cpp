/**
* @file Scene.cpp
*/
#include "Scene.h"
#include "Graphics.h"
#include <algorithm>

namespace Scene {

namespace /* unnamed */ {

/**
* 遷移元シーンIDの小なり比較.
*/
struct LessCurrentSceneId
{
	bool operator()(const Transition& lhs, int rhs) const { return lhs.currentScene < rhs; }
	bool operator()(int lhs, const Transition& rhs) const { return lhs < rhs.currentScene; }
	bool operator()(const Transition& lhs, const Transition& rhs) const { return lhs.currentScene < rhs.currentScene; }
};

} // unnamed namespace

/**
* 遷移制御クラスの初期化.
*
* @param transitionList  遷移情報の配列へのポインタ.
* @param transitionCount 遷移情報の数.
* @param creatorList     作成情報の配列へのポインタ.
* @param creatorCount    作成情報の数.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool TransitionController::Initialize(const Transition* transitionList, size_t transitionCount, const Creator* creatorList, size_t creatorCount)
{
	transitionMap.assign(transitionList, transitionList + transitionCount);
	std::sort(transitionMap.begin(), transitionMap.end(), [](const Transition& lhs, const Transition& rhs) { return lhs.currentScene < rhs.currentScene; });
	
	creatorMap.assign(creatorList, creatorList + creatorCount);
	std::sort(creatorMap.begin(), creatorMap.end(), [](const Creator& lhs, const Creator& rhs) { return lhs.id < rhs.id; });
	return true;
}

/**
* 遷移を開始する.
*
* @param startSceneId 最初に実行するシーンのID.
*
* @retval true  正常に開始された.
* @retval false 開始に失敗した.
*/
bool TransitionController::Start(Context& context, int startSceneId)
{
	if (const Creator* creator = FindCreator(startSceneId)) {
		LoadScene(context, creator);
		return true;
	}
	return false;
}


/**
* 遷移を終了する.
*/
void TransitionController::Stop(Context& context)
{
	while (!sceneStack.empty()) {
		UnloadScene(context);
	}
}

/**
* シーンを更新する.
*
* @param delta 経過時間(秒).
*/
void TransitionController::Update(Context& context, double delta)
{
	if (sceneStack.empty()) {
		return;
	}
	const auto itrEndPausedScene = sceneStack.end() - 1;
	for (auto itr = sceneStack.begin(); itr != itrEndPausedScene; ++itr) {
		itr->p->UpdateForPause(context, delta);
	}
	const int exitCode = sceneStack.back().p->Update(context, delta);
	if (exitCode == Scene::ExitCode_Continue) {
		return;
	}
	const auto range = std::equal_range(transitionMap.begin(), transitionMap.end(), sceneStack.back().id, LessCurrentSceneId());
	const auto itr = std::find_if(range.first, range.second,
		[exitCode](const Transition& trans) { return trans.trans.exitCode == exitCode; });
	if (itr != range.second) {
		switch (itr->trans.type) {
		case TransitionType::Jump:
			if (const Creator* creator = FindCreator(itr->trans.nextScene)) {
				UnloadScene(context);
				LoadScene(context, creator);
			}
			break;
		case TransitionType::Push:
			if (const Creator* creator = FindCreator(itr->trans.nextScene)) {
				sceneStack.back().p->Pause(context);
				LoadScene(context, creator);
			}
			break;
		case TransitionType::Pop:
			UnloadScene(context);
			sceneStack.back().p->Resume(context);
			break;
		}
	}
	Graphics::Graphics::Get().texMap.GC();
}

/**
* シーンを描画する.
*
* @param graphics 描画情報.
*/
void TransitionController::Draw(Graphics::Graphics& graphics) const
{
	for (const auto& scene : sceneStack) {
		if (scene.p->GetState() == Scene::StatusCode::Runnable) {
			scene.p->Draw(graphics);
		}
	}
}

/**
* シーン作成情報を検索する.
*
* @param sceneId 検索するシーンID.
*
* @retval nullptr以外 sceneIdに対応する作成情報へのポインタ.
* @retval nullptr     sceneIdに対応する作成情報は存在しない.
*/
const Creator* TransitionController::FindCreator(int sceneId) const
{
	const auto itr = std::lower_bound(creatorMap.begin(), creatorMap.end(), sceneId,
		[](const Creator& lhs, int rhs) { return lhs.id < rhs; }
	);
	if (itr == creatorMap.end() || itr->id != sceneId) {
		return nullptr;
	}
	return &*itr;
}

/**
* 新しいシーンを開始する.
*
* @param creator 開始するシーンの作成情報へのポインタ.
*/
void TransitionController::LoadScene(Context& context, const Creator* creator)
{
	sceneStack.push_back({ creator->id, creator->func() });
	sceneStack.back().p->Load(context);
	sceneStack.back().p->status = Scene::StatusCode::Runnable;
}

/**
* シーンを終了する.
*/
void TransitionController::UnloadScene(Context& context)
{
	sceneStack.back().p->status = Scene::StatusCode::Unloading;
	sceneStack.back().p->Unload(context);
	sceneStack.back().p->status = Scene::StatusCode::Stopped;
	sceneStack.pop_back();
	Graphics::Graphics::Get().WaitForGpu();
}

} // namespace Scene