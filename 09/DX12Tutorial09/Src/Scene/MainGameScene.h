/**
* @file MainGameScene.h
*/
#ifndef DX12TUTORIAL_SRC_SCENE_MAINGAMESCENE_H_
#define DX12TUTORIAL_SRC_SCENE_MAINGAMESCENE_H_
#include "../Scene.h"
#include "../Texture.h"
#include "../Sprite.h"
#include "../Animation.h"
#include "../Action.h"

class MainGameScene : public Scene::Scene
{
public:
	enum ExitCode
	{
		ExitCode_Ending = ExitCode_User,
		ExitCode_Pause,
		ExitCode_GameOver,
	};

	static ::Scene::ScenePtr Create();

	virtual bool Load() override;
	virtual bool Unload() override;
	virtual int Update(double delta) override;
	virtual void Draw(Graphics::Graphics& graphics) const override;

	struct Occurrence;
	struct FormationData;

private:
	MainGameScene();
	MainGameScene(const MainGameScene&) = delete;
	MainGameScene& operator=(const MainGameScene&) = delete;

	Resource::Texture texBackground;
	Resource::Texture texObjects;
	Resource::Texture texFont;
	std::vector<Sprite::Sprite> sprBackground;
	std::vector<Sprite::Sprite> sprPlayer;
	std::vector<Sprite::Sprite> sprEnemy;
	std::vector<Sprite::Sprite> sprFont;
	Sprite::FilePtr cellFile[2];
	Sprite::FilePtr cellPlayer;
	AnimationFile anmObjects;
	AnimationFile anmOthers;
	Action::FilePtr actionFile;
	const Occurrence* pCurOccurrence;
	const Occurrence* pEndOccurrence;

	struct Formation {
		float time;
		DirectX::XMFLOAT3 pos;
		const FormationData* cur;
		const FormationData* end;
	};
	std::vector<Formation> formationList;
	std::vector<Sprite::Sprite*> freePlayerShotList;
	std::vector<Sprite::Sprite*> freeEnemyList;
	std::vector<Sprite::Sprite*> freeEnemyShotList;
	double time;

	int playerShotCycle = 0;
	float playerShotInterval = 0;
};

#endif // DX12TUTORIAL_SRC_SCENE_MAINGAMESCENE_H_
