/**
* @file Scene.cpp
*/
#include "Scene.h"
#include "Graphics.h"
#include <algorithm>

namespace Scene {

namespace /* unnamed */ {

/**
* �J�ڌ��V�[��ID�̏��Ȃ��r.
*/
struct LessCurrentSceneId
{
	bool operator()(const Transition& lhs, int rhs) const { return lhs.currentScene < rhs; }
	bool operator()(int lhs, const Transition& rhs) const { return lhs < rhs.currentScene; }
	bool operator()(const Transition& lhs, const Transition& rhs) const { return lhs.currentScene < rhs.currentScene; }
};

} // unnamed namespace

/**
* �J�ڐ���N���X�̏�����.
*
* @param transitionList  �J�ڏ��̔z��ւ̃|�C���^.
* @param transitionCount �J�ڏ��̐�.
* @param creatorList     �쐬���̔z��ւ̃|�C���^.
* @param creatorCount    �쐬���̐�.
*
* @retval true  ����������.
* @retval false ���������s.
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
* �J�ڂ��J�n����.
*
* @param startSceneId �ŏ��Ɏ��s����V�[����ID.
*
* @retval true  ����ɊJ�n���ꂽ.
* @retval false �J�n�Ɏ��s����.
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
* �J�ڂ��I������.
*/
void TransitionController::Stop()
{
	const auto rend = sceneStack.rend();
	for (auto ri = sceneStack.rbegin(); ri != rend; ++ri) {
		ri->p->Unload();
	}
}

/**
* �V�[�����X�V����.
*
* @param delta �o�ߎ���(�b).
*/
void TransitionController::Update(double delta)
{
	if (sceneStack.empty()) {
		return;
	}
	const auto itrEndPausedScene = sceneStack.end() - 1;
	for (auto itr = sceneStack.begin(); itr != itrEndPausedScene; ++itr) {
		itr->p->UpdateForPause(delta);
	}
	const int exitCode = sceneStack.back().p->Update(delta);
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
	Graphics::Graphics::Get().texMap.GC();
}

/**
* �V�[����`�悷��.
*
* @param graphics �`����.
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
* �V�[���쐬������������.
*
* @param sceneId ��������V�[��ID.
*
* @retval nullptr�ȊO sceneId�ɑΉ�����쐬���ւ̃|�C���^.
* @retval nullptr     sceneId�ɑΉ�����쐬���͑��݂��Ȃ�.
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
* �V�����V�[�����J�n����.
*
* @param creator �J�n����V�[���̍쐬���ւ̃|�C���^.
*/
void TransitionController::LoadScene(const Creator* creator)
{
	sceneStack.push_back({ creator->id, creator->func() });
	sceneStack.back().p->Load();
	sceneStack.back().p->status = Scene::StatusCode::Runnable;
}

/**
* �V�[�����I������.
*/
void TransitionController::UnloadScene()
{
	sceneStack.back().p->status = Scene::StatusCode::Unloading;
	sceneStack.back().p->Unload();
	sceneStack.back().p->status = Scene::StatusCode::Stopped;
	sceneStack.pop_back();
	Graphics::Graphics::Get().WaitForGpu();
}

} // namespace Scene